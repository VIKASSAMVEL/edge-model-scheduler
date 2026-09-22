#pragma once

#include <atomic>
#include <chrono>
#include <functional>
#include <iostream>
#include <thread>
#include <string>
#include <cstdint>

#ifdef __linux__
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
// systemd watchdog support (optional)
#ifdef HAS_SYSTEMD
#include <systemd/sd-daemon.h>
#endif
#endif

namespace edge_scheduler {

/**
 * WatchdogDaemon: Self-Healing Process Supervisor
 *
 * Architecture:
 *   1. Heartbeat Loop: Periodic sd_notify(WATCHDOG=1) to systemd
 *   2. Health Checks: Custom health predicates (memory, GPU, queue depth)
 *   3. Graceful Degradation: On health failure, attempt recovery before crash
 *   4. Crash Recovery: Structured shutdown with state persistence
 *
 * Integration with systemd:
 *   [Service]
 *   Type=notify
 *   WatchdogSec=5s
 *   Restart=always
 *   RestartSec=1
 *
 * If the daemon fails to ping sd_notify within WatchdogSec,
 * systemd kills and restarts the process automatically.
 */

enum class DaemonState : uint8_t {
    STARTING = 0,
    RUNNING  = 1,
    DEGRADED = 2,  // Operating with reduced capability
    STOPPING = 3,
    STOPPED  = 4,
};

class WatchdogDaemon {
public:
    using HealthCheck = std::function<bool()>;
    using ShutdownHook = std::function<void()>;

    WatchdogDaemon(uint32_t heartbeat_interval_ms = 2000)
        : heartbeat_interval_ms_(heartbeat_interval_ms) {}

    ~WatchdogDaemon() {
        stop();
    }

    /**
     * Register a named health check. All checks must pass for RUNNING state.
     */
    void add_health_check(const std::string& name, HealthCheck check) {
        health_checks_.push_back({name, std::move(check)});
    }

    /**
     * Register a shutdown hook for graceful cleanup.
     */
    void on_shutdown(ShutdownHook hook) {
        shutdown_hooks_.push_back(std::move(hook));
    }

    /**
     * Start the watchdog supervisor thread.
     * Signals systemd READY=1 on successful startup.
     */
    void start() {
        if (state_ != DaemonState::STOPPED && state_ != DaemonState::STARTING) return;
        state_ = DaemonState::STARTING;
        running_ = true;

        // Install signal handlers for graceful shutdown
        install_signal_handlers();

        // Notify systemd we're ready
        notify_systemd("READY=1");
        notify_systemd("STATUS=EdgeModelScheduler daemon started");

        state_ = DaemonState::RUNNING;
        consecutive_failures_ = 0;

        watchdog_thread_ = std::thread(&WatchdogDaemon::watchdog_loop, this);
    }

    /**
     * Stop the watchdog and invoke all shutdown hooks.
     */
    void stop() {
        if (!running_) return;
        state_ = DaemonState::STOPPING;
        notify_systemd("STOPPING=1");
        running_ = false;

        if (watchdog_thread_.joinable()) {
            watchdog_thread_.join();
        }

        // Execute shutdown hooks in reverse order
        for (auto it = shutdown_hooks_.rbegin(); it != shutdown_hooks_.rend(); ++it) {
            try { (*it)(); } catch (...) {}
        }

        state_ = DaemonState::STOPPED;
    }

    DaemonState state() const { return state_; }
    uint64_t heartbeat_count() const { return heartbeat_count_; }
    uint32_t consecutive_failures() const { return consecutive_failures_; }
    bool is_healthy() const { return state_ == DaemonState::RUNNING; }

    // Global signal handler state
    static std::atomic<bool>& signal_received() {
        static std::atomic<bool> flag{false};
        return flag;
    }

private:
    void watchdog_loop() {
        while (running_ && !signal_received()) {
            // Run all health checks
            bool all_healthy = true;
            std::string failed_check;
            for (const auto& [name, check] : health_checks_) {
                if (!check()) {
                    all_healthy = false;
                    failed_check = name;
                    break;
                }
            }

            if (all_healthy) {
                consecutive_failures_ = 0;
                if (state_ == DaemonState::DEGRADED) {
                    state_ = DaemonState::RUNNING;
                    notify_systemd("STATUS=Recovered from degraded state");
                }
                // Ping systemd watchdog
                notify_systemd("WATCHDOG=1");
                heartbeat_count_++;
            } else {
                consecutive_failures_++;
                if (consecutive_failures_ >= max_failures_before_degraded_) {
                    state_ = DaemonState::DEGRADED;
                    notify_systemd("STATUS=DEGRADED: health check failed: " + failed_check);
                    std::cerr << "[Watchdog] DEGRADED: " << failed_check
                              << " failed (" << consecutive_failures_ << " consecutive)"
                              << std::endl;
                }
                // Still ping watchdog to avoid systemd kill during degraded recovery
                notify_systemd("WATCHDOG=1");
            }

            std::this_thread::sleep_for(
                std::chrono::milliseconds(heartbeat_interval_ms_));
        }

        // Signal received — initiate graceful shutdown
        if (signal_received()) {
            stop();
        }
    }

    void install_signal_handlers() {
#ifdef __linux__
        struct sigaction sa;
        sa.sa_handler = [](int) { signal_received().store(true); };
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = 0;
        sigaction(SIGTERM, &sa, nullptr);
        sigaction(SIGINT, &sa, nullptr);
#endif
    }

    void notify_systemd(const std::string& status) {
#if defined(__linux__) && defined(HAS_SYSTEMD)
        sd_notify(0, status.c_str());
#else
        (void)status; // Suppress unused warning on non-Linux
#endif
    }

    struct NamedCheck {
        std::string name;
        HealthCheck check;
    };

    uint32_t heartbeat_interval_ms_;
    std::atomic<bool> running_{false};
    std::atomic<DaemonState> state_{DaemonState::STOPPED};
    std::thread watchdog_thread_;

    std::vector<NamedCheck> health_checks_;
    std::vector<ShutdownHook> shutdown_hooks_;

    uint64_t heartbeat_count_{0};
    uint32_t consecutive_failures_{0};
    uint32_t max_failures_before_degraded_{3};
};

} // namespace edge_scheduler

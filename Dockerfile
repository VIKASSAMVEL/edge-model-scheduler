FROM ubuntu:22.04 AS builder

ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    python3-dev \
    pybind11-dev \
    python3-pip \
    libpthread-stubs0-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /src
COPY edge-model-scheduler/ /src/

RUN mkdir -p build && cd build && \
    cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS="-O3 -DNDEBUG" && \
    make -j$(nproc) && \
    cpack -G DEB

# Runtime Image
FROM ubuntu:22.04 AS runtime

LABEL org.opencontainers.image.title="EdgeModelScheduler" \
      org.opencontainers.image.description="Ultra-Low-Latency Deterministic Co-Scheduler for Edge AI & Autonomous Robotics" \
      org.opencontainers.image.version="3.3.0" \
      org.opencontainers.image.authors="Vikas Samvel <engineering@vikassamvel.dev>" \
      org.opencontainers.image.source="https://github.com/VIKASSAMVEL/edge-model-scheduler" \
      org.opencontainers.image.licenses="Apache-2.0"

ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y \
    libc6 \
    libstdc++6 \
    ca-certificates \
    curl \
    && rm -rf /var/lib/apt/lists/*

COPY --from=builder /src/build/*.deb /tmp/
RUN dpkg -i /tmp/*.deb && rm -f /tmp/*.deb

EXPOSE 9090 9191

ENTRYPOINT ["edge-scheduler-daemon"]

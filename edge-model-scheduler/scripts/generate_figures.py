#!/usr/bin/env python3
"""
Publication-Quality Figure Generator for EdgeModelScheduler Proof Battery
Generates publication-ready SVG and PNG charts from raw benchmark CSV data.
Standard: IEEE / ACM EuroSys formatting (Colorblind-safe palette, clean typography).
"""

import os
import sys
import pandas as pd
import numpy as np
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt

# Styling configuration
plt.rcParams['font.family'] = 'sans-serif'
plt.rcParams['font.size'] = 11
plt.rcParams['axes.linewidth'] = 1.2
plt.rcParams['grid.alpha'] = 0.3
plt.rcParams['grid.linestyle'] = '--'

PALETTE = {
    'Sequential': '#8c564b',
    'Concurrent': '#e377c2',
    'RoundRobin': '#7f7f7f',
    'StaticPriority': '#bcbd22',
    'EDF': '#17becf',
    'DynamicBatcher': '#d62728',
    'EdgeModelScheduler': '#1f77b4'
}

def plot_p99_comparison(summary_csv: str, output_dir: str):
    if not os.path.exists(summary_csv):
        print(f"File not found: {summary_csv}")
        return
    df = pd.read_csv(summary_csv)
    
    # Simplify baseline names
    df['Label'] = df['Baseline'].apply(lambda x: x.replace('Baseline-', '').replace('-Ours', ''))
    
    fig, ax = plt.subplots(figsize=(10, 5), dpi=300)
    x = np.arange(len(df))
    width = 0.35
    
    rects1 = ax.bar(x - width/2, df['P99_ms'], width, label='Overall P99 Latency', color='#ff7f0e', alpha=0.85, edgecolor='black')
    rects2 = ax.bar(x + width/2, df['CriticalP99_ms'], width, label='Critical Safety P99', color='#1f77b4', alpha=0.85, edgecolor='black')
    
    ax.set_yscale('log')
    ax.set_ylabel('Latency (ms, Log Scale)')
    ax.set_title('P99 Tail Latency Comparison Under Infeasible Load (140% Saturation)', fontweight='bold', pad=12)
    ax.set_xticks(x)
    ax.set_xticklabels(df['Label'], rotation=25, ha='right', fontsize=9)
    ax.legend(frameon=True, facecolor='white', framealpha=0.9)
    ax.grid(True, which="both", ls="--", alpha=0.3)
    
    # Add horizontal safety budget line
    ax.axhline(y=15.0, color='red', linestyle=':', linewidth=1.5, label='Safety SLO (15ms)')
    
    plt.tight_layout()
    plt.savefig(os.path.join(output_dir, 'p99_tail_latency_comparison.png'))
    plt.savefig(os.path.join(output_dir, 'p99_tail_latency_comparison.svg'))
    plt.close()
    print("  [OK] Generated: p99_tail_latency_comparison")

def plot_deadline_miss_rates(summary_csv: str, output_dir: str):
    if not os.path.exists(summary_csv):
        return
    df = pd.read_csv(summary_csv)
    df['Label'] = df['Baseline'].apply(lambda x: x.replace('Baseline-', '').replace('-Ours', ''))
    
    fig, ax = plt.subplots(figsize=(9, 4.8), dpi=300)
    x = np.arange(len(df))
    width = 0.35
    
    ax.bar(x - width/2, df['MissRatePct'], width, label='Overall Miss Rate (%)', color='#d62728', alpha=0.85, edgecolor='black')
    ax.bar(x + width/2, df['CriticalMissRatePct'].clip(upper=100.0), width, label='Critical Safety Miss Rate (%)', color='#9467bd', alpha=0.85, edgecolor='black')
    
    ax.set_ylabel('Deadline Miss Rate (%)')
    ax.set_title('Deadline Miss Rates Under System Overload', fontweight='bold', pad=12)
    ax.set_xticks(x)
    ax.set_xticklabels(df['Label'], rotation=25, ha='right', fontsize=9)
    ax.set_ylim(0, 105)
    ax.legend(frameon=True, facecolor='white', framealpha=0.9)
    ax.grid(True, alpha=0.3)
    
    plt.tight_layout()
    plt.savefig(os.path.join(output_dir, 'deadline_miss_rates.png'))
    plt.savefig(os.path.join(output_dir, 'deadline_miss_rates.svg'))
    plt.close()
    print("  [OK] Generated: deadline_miss_rates")

def plot_mission_utility(summary_csv: str, output_dir: str):
    if not os.path.exists(summary_csv):
        return
    df = pd.read_csv(summary_csv)
    df['Label'] = df['Baseline'].apply(lambda x: x.replace('Baseline-', '').replace('-Ours', ''))
    
    fig, ax = plt.subplots(figsize=(9, 4.5), dpi=300)
    colors = ['#2ca02c' if u > 0 else '#d62728' for u in df['TotalUtility']]
    bars = ax.bar(df['Label'], df['TotalUtility'], color=colors, alpha=0.85, edgecolor='black')
    
    ax.axhline(0, color='black', linewidth=1)
    ax.set_ylabel('Net Mission Utility Score $\\mathcal{U}$')
    ax.set_title('Total Mission Utility Under Resource Contention', fontweight='bold', pad=12)
    ax.set_xticklabels(df['Label'], rotation=25, ha='right', fontsize=9)
    ax.grid(True, alpha=0.3)
    
    # Add values on top of bars
    for bar in bars:
        height = bar.get_height()
        y_pos = height + 30 if height >= 0 else height - 70
        ax.annotate(f'{int(height)}',
                    xy=(bar.get_x() + bar.get_width() / 2, y_pos),
                    xytext=(0, 0), textcoords="offset points",
                    ha='center', va='bottom', fontsize=9, fontweight='bold')
    
    plt.tight_layout()
    plt.savefig(os.path.join(output_dir, 'mission_utility_comparison.png'))
    plt.savefig(os.path.join(output_dir, 'mission_utility_comparison.svg'))
    plt.close()
    print("  [OK] Generated: mission_utility_comparison")

def plot_ablation_study(ablation_csv: str, output_dir: str):
    if not os.path.exists(ablation_csv):
        return
    df = pd.read_csv(ablation_csv)
    
    fig, ax1 = plt.subplots(figsize=(9, 4.8), dpi=300)
    
    x = np.arange(len(df))
    width = 0.38
    
    color = '#1f77b4'
    ax1.set_xlabel('Subsystem Configuration')
    ax1.set_ylabel('P99 Latency (ms)', color=color)
    bars1 = ax1.bar(x - width/2, df['P99_ms'], width, label='P99 Latency', color=color, alpha=0.8, edgecolor='black')
    ax1.tick_params(axis='y', labelcolor=color)
    ax1.set_yscale('log')
    
    ax2 = ax1.twinx()
    color = '#d62728'
    ax2.set_ylabel('Deadline Miss Rate (%)', color=color)
    bars2 = ax2.bar(x + width/2, df['DeadlineMissRatePct'], width, label='Miss Rate (%)', color=color, alpha=0.8, edgecolor='black')
    ax2.tick_params(axis='y', labelcolor=color)
    ax2.set_ylim(0, 105)
    
    ax1.set_xticks(x)
    ax1.set_xticklabels(df['Variant'], rotation=20, ha='right', fontsize=9)
    plt.title('Subsystem Ablation Study: Quantitative Contribution of Core Modules', fontweight='bold', pad=12)
    
    plt.tight_layout()
    plt.savefig(os.path.join(output_dir, 'ablation_study_breakdown.png'))
    plt.savefig(os.path.join(output_dir, 'ablation_study_breakdown.svg'))
    plt.close()
    print("  [OK] Generated: ablation_study_breakdown")

def main():
    results_dir = sys.argv[1] if len(sys.argv) > 1 else 'results'
    plots_dir = os.path.join(results_dir, 'plots')
    os.makedirs(plots_dir, exist_ok=True)
    
    summary_csv = os.path.join(results_dir, 'summary_metrics.csv')
    ablation_csv = os.path.join(results_dir, 'ablation_metrics.csv')
    
    print(f"\n[Figure Generator] Rendering publication-quality charts into: {plots_dir}")
    plot_p99_comparison(summary_csv, plots_dir)
    plot_deadline_miss_rates(summary_csv, plots_dir)
    plot_mission_utility(summary_csv, plots_dir)
    plot_ablation_study(ablation_csv, plots_dir)
    print("[SUCCESS] All publication figures generated successfully.\n")

if __name__ == '__main__':
    main()

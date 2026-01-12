#!/usr/bin/env python3
"""Plot multi-threading benchmark results"""

import matplotlib.pyplot as plt
import pandas as pd
import numpy as np

# Read the CSV data
data = pd.read_csv('benchmark_results.csv')

# Create a figure with 4 subplots
fig = plt.figure(figsize=(14, 10))
fig.suptitle('Multi-Threading Performance Analysis - Fraglets (sort.fra)', fontsize=16, fontweight='bold')

# Plot 1: Time vs Threads
ax1 = plt.subplot(2, 2, 1)
ax1.plot(data['Threads'], data['Time_ms'], 'o-', linewidth=2, markersize=8, color='#e74c3c')
ax1.set_xlabel('Number of Threads', fontsize=11)
ax1.set_ylabel('Execution Time (ms)', fontsize=11)
ax1.set_title('Execution Time vs Thread Count', fontsize=12, fontweight='bold')
ax1.grid(True, alpha=0.3)
ax1.set_xticks(data['Threads'])

# Annotate points
for i, row in data.iterrows():
    ax1.annotate(f'{row["Time_ms"]:.0f}ms',
                xy=(row['Threads'], row['Time_ms']),
                xytext=(0, 10), textcoords='offset points',
                ha='center', fontsize=9, alpha=0.7)

# Plot 2: Speedup vs Threads
ax2 = plt.subplot(2, 2, 2)
ax2.plot(data['Threads'], data['Speedup'], 'o-', linewidth=2, markersize=8,
         color='#3498db', label='Actual Speedup')
# Ideal speedup line
ideal_threads = np.linspace(1, max(data['Threads']), 100)
ax2.plot(ideal_threads, ideal_threads, '--', linewidth=2, color='#2ecc71',
         label='Ideal Speedup', alpha=0.7)
ax2.set_xlabel('Number of Threads', fontsize=11)
ax2.set_ylabel('Speedup', fontsize=11)
ax2.set_title('Speedup vs Thread Count', fontsize=12, fontweight='bold')
ax2.grid(True, alpha=0.3)
ax2.legend(loc='upper left')
ax2.set_xticks(data['Threads'])

# Plot 3: Efficiency vs Threads
ax3 = plt.subplot(2, 2, 3)
ax3.plot(data['Threads'], data['Efficiency_percent'], 'o-', linewidth=2,
         markersize=8, color='#9b59b6')
ax3.set_xlabel('Number of Threads', fontsize=11)
ax3.set_ylabel('Parallel Efficiency (%)', fontsize=11)
ax3.set_title('Parallel Efficiency vs Thread Count', fontsize=12, fontweight='bold')
ax3.grid(True, alpha=0.3)
ax3.set_xticks(data['Threads'])
ax3.axhline(y=100, color='gray', linestyle='--', alpha=0.5, label='100% Efficiency')
ax3.legend()

# Plot 4: Bar chart comparison
ax4 = plt.subplot(2, 2, 4)
x_pos = np.arange(len(data))
bars = ax4.bar(x_pos, data['Time_ms'], color='#e67e22', alpha=0.7)
ax4.set_xlabel('Number of Threads', fontsize=11)
ax4.set_ylabel('Execution Time (ms)', fontsize=11)
ax4.set_title('Execution Time Comparison', fontsize=12, fontweight='bold')
ax4.set_xticks(x_pos)
ax4.set_xticklabels(data['Threads'])
ax4.grid(True, alpha=0.3, axis='y')

# Add value labels on bars
for i, (bar, time) in enumerate(zip(bars, data['Time_ms'])):
    height = bar.get_height()
    ax4.text(bar.get_x() + bar.get_width()/2., height,
            f'{time:.0f}ms\n({data["Speedup"].iloc[i]:.3f}x)',
            ha='center', va='bottom', fontsize=8)

plt.tight_layout()
plt.savefig('benchmark_plot.png', dpi=150, bbox_inches='tight')
print("Plot saved to: benchmark_plot.png")

# Also create a simple ASCII chart for terminal viewing
print("\n" + "="*60)
print("PERFORMANCE SUMMARY")
print("="*60)
print(f"{'Threads':<10} {'Time (ms)':<12} {'Speedup':<12} {'Efficiency':<12}")
print("-"*60)
for _, row in data.iterrows():
    print(f"{int(row['Threads']):<10} {row['Time_ms']:<12.1f} "
          f"{row['Speedup']:<12.3f} {row['Efficiency_percent']:<12.1f}%")
print("="*60)

# Create a simple text-based graph
print("\nExecution Time Graph (each * = ~100ms):")
print("-"*60)
for _, row in data.iterrows():
    stars = int(row['Time_ms'] / 100)
    print(f"{int(row['Threads'])} threads: {'*' * stars} {row['Time_ms']:.0f}ms")
print("-"*60)

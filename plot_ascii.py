#!/usr/bin/env python3
"""Simple ASCII visualization of benchmark results"""

import csv

# Read CSV data
data = []
with open('benchmark_results.csv', 'r') as f:
    reader = csv.DictReader(f)
    for row in reader:
        data.append({
            'threads': int(row['Threads']),
            'time_ms': float(row['Time_ms']),
            'speedup': float(row['Speedup']),
            'efficiency': float(row['Efficiency_percent'])
        })

print("\n" + "="*70)
print("MULTI-THREADING PERFORMANCE ANALYSIS")
print("Workload: sort.fra | Iterations: 100,000 | Molecule Cap: 2,000")
print("="*70)

# Table view
print("\n{:<10} {:<15} {:<15} {:<15}".format("Threads", "Time (ms)", "Speedup", "Efficiency"))
print("-"*70)
for row in data:
    print("{:<10} {:<15.1f} {:<15.3f} {:<15.1f}%".format(
        row['threads'], row['time_ms'], row['speedup'], row['efficiency']))
print("="*70)

# ASCII Bar Chart - Execution Time
print("\nEXECUTION TIME CHART")
print("="*70)
max_time = max(d['time_ms'] for d in data)
scale = 50.0 / max_time  # Scale to 50 characters max

for row in data:
    bar_length = int(row['time_ms'] * scale)
    bar = '█' * bar_length
    print(f"{row['threads']} thread{'s' if row['threads'] > 1 else ' '}: {bar} {row['time_ms']:.0f}ms")

# ASCII Bar Chart - Speedup
print("\n" + "="*70)
print("SPEEDUP CHART (Ideal = 1.0x for 1 thread, 2.0x for 2 threads, etc.)")
print("="*70)

for row in data:
    ideal_speedup = row['threads']
    actual_bar_length = int(row['speedup'] * 10)  # 10 chars per 1x speedup
    ideal_bar_length = int(ideal_speedup * 10)

    actual_bar = '█' * max(1, actual_bar_length)
    ideal_bar = '░' * ideal_bar_length

    print(f"{row['threads']} thread{'s' if row['threads'] > 1 else ' '}: "
          f"{actual_bar}{' ' * (ideal_bar_length - actual_bar_length)} {row['speedup']:.3f}x "
          f"(ideal: {ideal_speedup:.1f}x)")

# Efficiency visualization
print("\n" + "="*70)
print("PARALLEL EFFICIENCY (% of ideal performance)")
print("="*70)

for row in data:
    eff_bar_length = int(row['efficiency'] / 2)  # Scale to 50 chars for 100%
    bar = '█' * eff_bar_length
    remainder = '░' * (50 - eff_bar_length)
    print(f"{row['threads']} thread{'s' if row['threads'] > 1 else ' '}: [{bar}{remainder}] {row['efficiency']:.1f}%")

print("\n" + "="*70)
print("ANALYSIS")
print("="*70)
print(f"Baseline (1 thread):  {data[0]['time_ms']:.1f} ms")
print(f"Best time achieved:   {data[0]['time_ms']:.1f} ms with 1 thread")
print(f"Worst time:           {data[-1]['time_ms']:.1f} ms with {data[-1]['threads']} threads")
print(f"Slowdown at 8 threads: {data[-1]['time_ms'] / data[0]['time_ms']:.1f}x slower")
print()
print("NOTE: Multi-threading shows negative scaling due to:")
print("  - Lock contention in fine-grained parallel operations")
print("  - Synchronization overhead in inject/expel operations")
print("  - Sequential dependencies in chemical reaction simulation")
print("  - Memory bandwidth contention across threads")
print("="*70)

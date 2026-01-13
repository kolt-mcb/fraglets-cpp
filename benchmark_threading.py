#!/usr/bin/env python3
"""
Multi-threaded benchmark testing actual C++ threading in fraglets.
Compares performance with 1, 2, 4, and 8 threads.
"""

import fraglets
import time
import random
import matplotlib.pyplot as plt
import numpy as np
from typing import List, Tuple

def generate_test_list(size: int, seed: int = 42) -> str:
    """Generate a random list of integers for sorting."""
    random.seed(seed)
    numbers = [random.randint(-1000, 1000) for _ in range(size)]
    return ' '.join(map(str, numbers))

def benchmark_with_threads(list_size: int, num_threads: int, max_iter: int = 100000, trials: int = 3) -> Tuple[float, float, int, bool]:
    """
    Benchmark with specified number of threads.
    Returns: (avg_time, std_time, avg_iterations, completed)
    """
    print(f"  {num_threads} threads: ", end='', flush=True)

    times = []
    iterations_list = []
    completed_all = True

    for trial in range(trials):
        # Create fresh fraglets instance
        frag = fraglets.fraglets()

        # Parse sort rules
        with open('sort.fra', 'r') as f:
            for line in f:
                line = line.strip()
                if line and not line.startswith('#') and not line.startswith('[psort 203'):
                    frag.parse(line)

        # Generate and inject test data
        test_data = generate_test_list(list_size, seed=trial)
        frag.parse(f"[psort {test_data}]")

        # Benchmark with threading
        start = time.perf_counter()
        if num_threads == 1:
            frag.run(max_iter, 5000, quiet=True)
        else:
            frag.run_parallel(max_iter, 5000, quiet=True, threads=num_threads)
        elapsed = time.perf_counter() - start

        times.append(elapsed)
        iterations_list.append(frag.iter)

        if frag.iter >= max_iter - 1:
            completed_all = False

    avg_time = np.mean(times)
    std_time = np.std(times)
    avg_iters = int(np.mean(iterations_list))

    print(f"{avg_time*1000:.3f}ms ± {std_time*1000:.3f}ms, {avg_iters} iters")

    return avg_time, std_time, avg_iters, completed_all

def create_threading_plots(results: dict):
    """Create visualization comparing performance across thread counts."""
    sizes = sorted(results.keys())
    thread_counts = [1, 2, 4, 8]

    fig, axes = plt.subplots(2, 2, figsize=(14, 10))
    fig.suptitle('Multi-threaded Parallel Sort: C++ Thread Pool Performance\n(Actual OS-Level Threading)',
                 fontsize=16, fontweight='bold')

    # Plot 1: Execution Time vs Thread Count
    ax1 = axes[0, 0]
    for size in sizes:
        times = [results[size][tc][0] * 1000 for tc in thread_counts]
        ax1.plot(thread_counts, times, marker='o', linewidth=2, markersize=8, label=f'{size} elements')
    ax1.set_xlabel('Number of Threads', fontsize=11, fontweight='bold')
    ax1.set_ylabel('Execution Time (ms)', fontsize=11, fontweight='bold')
    ax1.set_title('Execution Time vs Thread Count', fontsize=13)
    ax1.set_xticks(thread_counts)
    ax1.legend()
    ax1.grid(True, alpha=0.3)

    # Plot 2: Speedup Factor
    ax2 = axes[0, 1]
    for size in sizes:
        baseline = results[size][1][0]
        speedups = [baseline / results[size][tc][0] if results[size][tc][0] > 0 else 0 for tc in thread_counts]
        ax2.plot(thread_counts, speedups, marker='s', linewidth=2, markersize=8, label=f'{size} elements')

    # Add ideal speedup line
    ax2.plot(thread_counts, thread_counts, 'k--', linewidth=2, label='Ideal (linear)', alpha=0.5)

    ax2.set_xlabel('Number of Threads', fontsize=11, fontweight='bold')
    ax2.set_ylabel('Speedup Factor', fontsize=11, fontweight='bold')
    ax2.set_title('Parallel Speedup (vs 1 thread)', fontsize=13)
    ax2.set_xticks(thread_counts)
    ax2.legend()
    ax2.grid(True, alpha=0.3)

    # Plot 3: Throughput Comparison
    ax3 = axes[1, 0]
    x = np.arange(len(thread_counts))
    width = 0.2
    for i, size in enumerate(sizes):
        throughputs = [size / results[size][tc][0] if results[size][tc][0] > 0 else 0 for tc in thread_counts]
        ax3.bar(x + i*width, throughputs, width, label=f'{size} elements', alpha=0.8)

    ax3.set_xlabel('Number of Threads', fontsize=11, fontweight='bold')
    ax3.set_ylabel('Throughput (elements/sec)', fontsize=11, fontweight='bold')
    ax3.set_title('Sorting Throughput', fontsize=13)
    ax3.set_xticks(x + width * 1.5)
    ax3.set_xticklabels(thread_counts)
    ax3.legend()
    ax3.ticklabel_format(axis='y', style='scientific', scilimits=(0,0))
    ax3.grid(axis='y', alpha=0.3)

    # Plot 4: Parallel Efficiency
    ax4 = axes[1, 1]
    for size in sizes:
        baseline = results[size][1][0]
        efficiencies = [(baseline / results[size][tc][0]) / tc * 100 if results[size][tc][0] > 0 else 0
                        for tc in thread_counts]
        ax4.plot(thread_counts, efficiencies, marker='d', linewidth=2, markersize=8, label=f'{size} elements')

    ax4.axhline(y=100, color='k', linestyle='--', linewidth=2, label='100% efficiency', alpha=0.5)
    ax4.set_xlabel('Number of Threads', fontsize=11, fontweight='bold')
    ax4.set_ylabel('Parallel Efficiency (%)', fontsize=11, fontweight='bold')
    ax4.set_title('Parallel Efficiency = (Speedup / Threads) × 100%', fontsize=13)
    ax4.set_xticks(thread_counts)
    ax4.set_ylim(0, 120)
    ax4.legend()
    ax4.grid(True, alpha=0.3)

    plt.tight_layout()
    plt.savefig('parallel_sort_benchmark.png', dpi=300, bbox_inches='tight')
    print(f"\n✓ Threading plot saved as 'parallel_sort_benchmark.png'")

def main():
    print("""
    ╔════════════════════════════════════════════════════════════════╗
    ║  Multi-threaded Parallel Sort Benchmark                       ║
    ║  Testing ACTUAL C++ thread pool with 1, 2, 4, and 8 threads   ║
    ╚════════════════════════════════════════════════════════════════╝
    """)

    # Test with different problem sizes
    test_sizes = [800, 1600, 3200]
    thread_counts = [1, 2, 4, 8]

    print(f"Problem sizes: {test_sizes}")
    print(f"Thread counts: {thread_counts}")
    print(f"Running 3 trials per configuration...\n")

    # Dictionary to store results: results[size][threads] = (time, std, iters, completed)
    results = {}

    for size in test_sizes:
        print(f"\n{'='*70}")
        print(f"Benchmarking {size} elements")
        print(f"{'='*70}")

        results[size] = {}

        for num_threads in thread_counts:
            try:
                avg_time, std_time, avg_iters, completed = benchmark_with_threads(
                    size, num_threads, max_iter=100000, trials=3
                )
                results[size][num_threads] = (avg_time, std_time, avg_iters, completed)
            except Exception as e:
                print(f"  ERROR: {e}")
                results[size][num_threads] = (0, 0, 100000, False)

    # Print summary table
    print(f"\n{'='*80}")
    print("PERFORMANCE SUMMARY")
    print(f"{'='*80}")

    for size in test_sizes:
        print(f"\n{size} elements:")
        print(f"{'Threads':<10} {'Time (ms)':<15} {'Iterations':<12} {'Speedup':<10} {'Efficiency':<12}")
        print(f"{'-'*70}")

        baseline_time = results[size][1][0]

        for tc in thread_counts:
            avg_t, std_t, iters, comp = results[size][tc]
            speedup = baseline_time / avg_t if avg_t > 0 else 0
            efficiency = (speedup / tc * 100) if tc > 0 else 0
            status = "✓" if comp else "✗"

            print(f"{tc:<10} {avg_t*1000:>7.3f} ± {std_t*1000:<5.3f} {iters:<12,} {speedup:<10.2f}x {efficiency:<10.1f}% {status}")

    # Create visualization
    print(f"\n{'='*80}")
    print("Generating threading performance plots...")
    print(f"{'='*80}")

    create_threading_plots(results)

    print(f"\n{'='*80}")
    print("BENCHMARK COMPLETE!")
    print(f"{'='*80}")
    print(f"✓ Tested {len(test_sizes)} problem sizes with {len(thread_counts)} thread configurations")
    print(f"✓ Used actual C++ std::thread pool for true parallel execution")
    print(f"✓ Measured real speedup from OS-level multi-threading")
    print(f"✓ Generated comprehensive threading performance visualization")

if __name__ == "__main__":
    main()

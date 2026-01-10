#include "fraglets.h"
#include <chrono>
#include <iostream>
#include <iomanip>
#include <vector>

std::string alphabet = {"abcdtuvxz"};

int main() {
    const int iterations = 10000;  // Fewer iterations since dataset is larger
    const int molCap = 10000;
    std::vector<int> thread_counts = {1, 2, 4, 6, 8};

    std::cout << "========================================================================" << std::endl;
    std::cout << "THREADING PERFORMANCE TEST - 200 Number Dataset" << std::endl;
    std::cout << "========================================================================" << std::endl;
    std::cout << "Dataset: 200 random numbers (-1000 to 1000)" << std::endl;
    std::cout << "Partition: 8-way split = ~25 numbers per chunk" << std::endl;
    std::cout << "Iterations: " << iterations << std::endl;
    std::cout << "========================================================================" << std::endl;
    std::cout << std::endl;

    std::cout << "Parallel Sort (parsort_xlarge.fra) with varying thread counts:" << std::endl;
    std::cout << "----------------------------------------------------------------------" << std::endl;

    std::vector<double> times;
    double baseline = 0;

    for (int threads : thread_counts) {
        std::cout << "  " << std::setw(2) << threads << " thread(s): " << std::flush;

        fraglets frag;
        frag.interpret("parsort_xlarge.fra");

        auto start = std::chrono::high_resolution_clock::now();
        frag.run(iterations, molCap, true, true, threads);
        auto end = std::chrono::high_resolution_clock::now();

        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        double time_ms = duration.count();
        times.push_back(time_ms);

        if (threads == 1) {
            baseline = time_ms;
            std::cout << std::setw(8) << std::fixed << std::setprecision(1) << time_ms << " ms (baseline)" << std::endl;
        } else {
            double speedup = baseline / time_ms;
            double efficiency = (speedup / threads) * 100.0;

            std::cout << std::setw(8) << std::fixed << std::setprecision(1) << time_ms << " ms";
            std::cout << "  │  speedup: " << std::fixed << std::setprecision(3) << speedup << "x";
            std::cout << "  │  efficiency: " << std::fixed << std::setprecision(1) << efficiency << "%";

            if (speedup > 1.05) {
                std::cout << "  ✓ FASTER!";
            } else if (speedup > 0.95) {
                std::cout << "  ≈ similar";
            } else {
                std::cout << "  ✗ slower";
            }
            std::cout << std::endl;
        }
    }

    std::cout << std::endl;
    std::cout << "========================================================================" << std::endl;
    std::cout << "ANALYSIS" << std::endl;
    std::cout << "========================================================================" << std::endl;

    // Find best configuration
    double best_time = times[0];
    int best_threads = thread_counts[0];
    double best_speedup = 1.0;

    for (size_t i = 1; i < times.size(); i++) {
        double speedup = baseline / times[i];
        if (times[i] < best_time) {
            best_time = times[i];
            best_threads = thread_counts[i];
            best_speedup = speedup;
        }
    }

    if (best_threads == 1) {
        std::cout << "Result: Single-threaded is still fastest" << std::endl;
        std::cout << "Overhead at " << thread_counts.back() << " threads: "
                  << std::fixed << std::setprecision(1)
                  << (times.back() / baseline - 1.0) * 100.0 << "%" << std::endl;
        std::cout << std::endl;
        std::cout << "Why threading doesn't help:" << std::endl;
        std::cout << "  - Lock contention in inject/expel operations" << std::endl;
        std::cout << "  - Thread creation/synchronization overhead" << std::endl;
        std::cout << "  - Chunk size (~25 numbers) is still too small" << std::endl;
        std::cout << "  - Each unimolecular operation completes too quickly" << std::endl;
    } else {
        std::cout << "✓ SUCCESS! Threading provides speedup!" << std::endl;
        std::cout << "  Best: " << best_threads << " threads" << std::endl;
        std::cout << "  Time: " << std::fixed << std::setprecision(1) << best_time << " ms" << std::endl;
        std::cout << "  Speedup: " << std::fixed << std::setprecision(3) << best_speedup << "x" << std::endl;
        std::cout << "  Efficiency: " << std::fixed << std::setprecision(1)
                  << (best_speedup / best_threads) * 100.0 << "%" << std::endl;
    }

    std::cout << "========================================================================" << std::endl;

    return 0;
}

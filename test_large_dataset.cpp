#include "fraglets.h"
#include <chrono>
#include <iostream>
#include <iomanip>
#include <vector>
#include <fstream>

std::string alphabet = {"abcdtuvxz"};

int main() {
    const int iterations = 50000;
    const int molCap = 5000;
    std::vector<int> thread_counts = {1, 2, 4, 8};

    std::cout << "========================================================================" << std::endl;
    std::cout << "LARGE DATASET BENCHMARK: Sequential vs Parallel Sort" << std::endl;
    std::cout << "========================================================================" << std::endl;
    std::cout << "Dataset Size: 100 numbers" << std::endl;
    std::cout << "Iterations: " << iterations << std::endl;
    std::cout << "Molecule Cap: " << molCap << std::endl;
    std::cout << "Partition: 8-way split for parallel sort" << std::endl;
    std::cout << "========================================================================" << std::endl;
    std::cout << std::endl;

    // Benchmark sequential sort with large dataset
    std::cout << "Testing SEQUENTIAL SORT (sort_large.fra) - O(n²) algorithm" << std::endl;
    std::cout << "----------------------------------------------------------------------" << std::endl;

    std::vector<double> seq_times;
    for (int threads : thread_counts) {
        std::cout << "  " << threads << " thread(s): " << std::flush;

        fraglets frag;
        frag.interpret("sort_large.fra");
        auto start = std::chrono::high_resolution_clock::now();
        frag.run(iterations, molCap, true, true, threads);
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        double time_ms = duration.count();
        seq_times.push_back(time_ms);

        std::cout << std::setw(8) << std::fixed << std::setprecision(1) << time_ms << " ms";
        if (threads > 1) {
            double speedup = seq_times[0] / time_ms;
            std::cout << " (speedup: " << std::fixed << std::setprecision(3) << speedup << "x)";
        }
        std::cout << std::endl;
    }

    std::cout << std::endl;

    // Benchmark parallel sort with large dataset
    std::cout << "Testing PARALLEL SORT (parsort_large.fra) - O(n log n) with partition" << std::endl;
    std::cout << "----------------------------------------------------------------------" << std::endl;

    std::vector<double> par_times;
    for (int threads : thread_counts) {
        std::cout << "  " << threads << " thread(s): " << std::flush;

        fraglets frag;
        frag.interpret("parsort_large.fra");
        auto start = std::chrono::high_resolution_clock::now();
        frag.run(iterations, molCap, true, true, threads);
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        double time_ms = duration.count();
        par_times.push_back(time_ms);

        std::cout << std::setw(8) << std::fixed << std::setprecision(1) << time_ms << " ms";
        if (threads > 1) {
            double speedup = par_times[0] / time_ms;
            std::cout << " (speedup: " << std::fixed << std::setprecision(3) << speedup << "x)";
        }
        std::cout << std::endl;
    }

    std::cout << std::endl;
    std::cout << "========================================================================" << std::endl;
    std::cout << "DETAILED COMPARISON" << std::endl;
    std::cout << "========================================================================" << std::endl;
    std::cout << std::setw(10) << "Threads"
              << std::setw(15) << "Sequential"
              << std::setw(15) << "Parallel"
              << std::setw(18) << "Algorithm Gain"
              << std::setw(18) << "Thread Scaling" << std::endl;
    std::cout << "------------------------------------------------------------------------" << std::endl;

    for (size_t i = 0; i < thread_counts.size(); i++) {
        double algo_improvement = (seq_times[i] - par_times[i]) / seq_times[i] * 100.0;
        double thread_speedup = par_times[0] / par_times[i];

        std::cout << std::setw(10) << thread_counts[i]
                  << std::setw(15) << std::fixed << std::setprecision(1) << seq_times[i]
                  << std::setw(15) << std::fixed << std::setprecision(1) << par_times[i]
                  << std::setw(17) << std::fixed << std::setprecision(1) << algo_improvement << "%"
                  << std::setw(17) << std::fixed << std::setprecision(3) << thread_speedup << "x"
                  << std::endl;
    }

    std::cout << std::endl;
    std::cout << "========================================================================" << std::endl;
    std::cout << "ANALYSIS" << std::endl;
    std::cout << "========================================================================" << std::endl;

    // Check for threading benefits
    bool threading_helps = false;
    double best_thread_speedup = 1.0;
    int best_thread_count = 1;

    for (size_t i = 1; i < par_times.size(); i++) {
        double speedup = par_times[0] / par_times[i];
        if (speedup > best_thread_speedup) {
            best_thread_speedup = speedup;
            best_thread_count = thread_counts[i];
        }
        if (speedup > 1.05) { // At least 5% speedup
            threading_helps = true;
        }
    }

    std::cout << "Algorithmic Improvement:" << std::endl;
    double overall_improvement = (seq_times[0] - par_times[0]) / seq_times[0] * 100.0;
    std::cout << "  parsort vs sort (1 thread): " << std::fixed << std::setprecision(1)
              << overall_improvement << "% faster" << std::endl;
    std::cout << std::endl;

    std::cout << "Threading Performance:" << std::endl;
    if (threading_helps) {
        std::cout << "  ✓ Threading DOES help!" << std::endl;
        std::cout << "  Best: " << best_thread_count << " threads with "
                  << std::fixed << std::setprecision(3) << best_thread_speedup << "x speedup" << std::endl;
    } else {
        std::cout << "  ✗ Threading still shows overhead" << std::endl;
        std::cout << "  Best: 1 thread (baseline)" << std::endl;
        std::cout << "  Overhead at " << thread_counts.back() << " threads: "
                  << std::fixed << std::setprecision(1)
                  << (par_times.back() / par_times[0] - 1.0) * 100.0 << "%" << std::endl;
    }

    std::cout << std::endl;
    std::cout << "Overall Winner:" << std::endl;

    // Find absolute best
    double best_time = seq_times[0];
    std::string best_config = "Sequential 1 thread";

    for (size_t i = 0; i < thread_counts.size(); i++) {
        if (seq_times[i] < best_time) {
            best_time = seq_times[i];
            best_config = "Sequential " + std::to_string(thread_counts[i]) + " threads";
        }
        if (par_times[i] < best_time) {
            best_time = par_times[i];
            best_config = "Parallel " + std::to_string(thread_counts[i]) + " threads";
        }
    }

    std::cout << "  " << best_config << ": " << std::fixed << std::setprecision(1)
              << best_time << " ms" << std::endl;

    // Save results to CSV
    std::ofstream csv("large_dataset_results.csv");
    csv << "Threads,Sequential_ms,Parallel_ms,AlgoImprovement_pct,ThreadSpeedup\n";
    for (size_t i = 0; i < thread_counts.size(); i++) {
        double algo_improvement = (seq_times[i] - par_times[i]) / seq_times[i] * 100.0;
        double thread_speedup = par_times[0] / par_times[i];
        csv << thread_counts[i] << ","
            << seq_times[i] << ","
            << par_times[i] << ","
            << algo_improvement << ","
            << thread_speedup << "\n";
    }
    csv.close();

    std::cout << std::endl;
    std::cout << "Results saved to: large_dataset_results.csv" << std::endl;
    std::cout << "========================================================================" << std::endl;

    return 0;
}

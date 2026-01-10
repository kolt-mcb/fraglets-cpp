#include "fraglets.h"
#include <chrono>
#include <iostream>
#include <iomanip>
#include <vector>

std::string alphabet = {"abcdtuvxz"};

double run_benchmark(const std::string& workload_file, const std::string& name,
                     int threads, int iterations, int molCap) {
    fraglets frag;
    frag.interpret(workload_file);

    auto start = std::chrono::high_resolution_clock::now();
    frag.run(iterations, molCap, true, true, threads);
    auto end = std::chrono::high_resolution_clock::now();

    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    return duration.count();
}

int main() {
    const int iterations = 100000;
    const int molCap = 2000;
    std::vector<int> thread_counts = {1, 2, 4, 8};

    std::cout << "========================================================================" << std::endl;
    std::cout << "COMPARISON: Sequential Sort vs Parallel Sort" << std::endl;
    std::cout << "========================================================================" << std::endl;
    std::cout << "Iterations: " << iterations << std::endl;
    std::cout << "Molecule Cap: " << molCap << std::endl;
    std::cout << "========================================================================" << std::endl;
    std::cout << std::endl;

    // Benchmark original sequential sort (sort.fra)
    std::cout << "Testing SEQUENTIAL SORT (sort.fra) - Original Algorithm" << std::endl;
    std::cout << "----------------------------------------------------------------------" << std::endl;

    std::vector<double> seq_times;
    for (int threads : thread_counts) {
        std::cout << "  " << threads << " thread(s): " << std::flush;

        fraglets frag;
        frag.interpret("sort.fra");
        auto start = std::chrono::high_resolution_clock::now();
        frag.run(iterations, molCap, true, true, threads);
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        double time_ms = duration.count();
        seq_times.push_back(time_ms);

        std::cout << std::setw(6) << time_ms << " ms";
        if (threads > 1) {
            double speedup = seq_times[0] / time_ms;
            std::cout << " (speedup: " << std::fixed << std::setprecision(2) << speedup << "x)";
        }
        std::cout << std::endl;
    }

    std::cout << std::endl;

    // Benchmark parallel sort (parsort.fra)
    std::cout << "Testing PARALLEL SORT (parsort.fra) - With partition/merge" << std::endl;
    std::cout << "----------------------------------------------------------------------" << std::endl;

    std::vector<double> par_times;
    for (int threads : thread_counts) {
        std::cout << "  " << threads << " thread(s): " << std::flush;

        fraglets frag;
        frag.interpret("parsort.fra");
        auto start = std::chrono::high_resolution_clock::now();
        frag.run(iterations, molCap, true, true, threads);
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        double time_ms = duration.count();
        par_times.push_back(time_ms);

        std::cout << std::setw(6) << time_ms << " ms";
        if (threads > 1) {
            double speedup = par_times[0] / time_ms;
            std::cout << " (speedup: " << std::fixed << std::setprecision(2) << speedup << "x)";
        }
        std::cout << std::endl;
    }

    std::cout << std::endl;
    std::cout << "========================================================================" << std::endl;
    std::cout << "COMPARISON SUMMARY" << std::endl;
    std::cout << "========================================================================" << std::endl;
    std::cout << std::setw(10) << "Threads"
              << std::setw(15) << "Sequential"
              << std::setw(15) << "Parallel"
              << std::setw(20) << "Improvement" << std::endl;
    std::cout << "------------------------------------------------------------------------" << std::endl;

    for (size_t i = 0; i < thread_counts.size(); i++) {
        double improvement = (seq_times[i] - par_times[i]) / seq_times[i] * 100.0;
        std::string result = improvement > 0 ?
            std::to_string((int)improvement) + "% faster" :
            std::to_string((int)-improvement) + "% slower";

        std::cout << std::setw(10) << thread_counts[i]
                  << std::setw(15) << std::fixed << std::setprecision(1) << seq_times[i]
                  << std::setw(15) << std::fixed << std::setprecision(1) << par_times[i]
                  << std::setw(20) << result << std::endl;
    }

    std::cout << std::endl;
    std::cout << "========================================================================" << std::endl;
    std::cout << "ANALYSIS" << std::endl;
    std::cout << "========================================================================" << std::endl;

    // Check if parallel is faster at any thread count
    bool any_faster = false;
    int best_thread_count = 1;
    double best_improvement = -1000;

    for (size_t i = 0; i < thread_counts.size(); i++) {
        double improvement = (seq_times[i] - par_times[i]) / seq_times[i] * 100.0;
        if (improvement > 0) {
            any_faster = true;
        }
        if (improvement > best_improvement) {
            best_improvement = improvement;
            best_thread_count = thread_counts[i];
        }
    }

    if (any_faster) {
        std::cout << "✓ Parallel sort IS faster!" << std::endl;
        std::cout << "  Best configuration: " << best_thread_count << " threads" << std::endl;
        std::cout << "  Improvement: " << std::fixed << std::setprecision(1)
                  << best_improvement << "%" << std::endl;
    } else {
        std::cout << "✗ Parallel sort is currently slower due to:" << std::endl;
        std::cout << "  - Lock contention in inject/expel operations" << std::endl;
        std::cout << "  - Synchronization overhead dominates for small workload" << std::endl;
        std::cout << "  - Fine-grained parallelism doesn't amortize thread creation" << std::endl;
        std::cout << std::endl;
        std::cout << "Least bad configuration: " << best_thread_count << " threads" << std::endl;
        std::cout << "Performance loss: " << std::fixed << std::setprecision(1)
                  << -best_improvement << "%" << std::endl;
    }

    std::cout << "========================================================================" << std::endl;

    return 0;
}

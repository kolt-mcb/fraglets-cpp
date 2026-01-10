#include "fraglets.h"
#include <chrono>
#include <iostream>
#include <iomanip>
#include <vector>
#include <fstream>

std::string alphabet = {"abcdtuvxz"};

int main() {
    const int iterations = 1000;  // Reduced iterations due to massive dataset
    const int molCap = 50000;     // Higher cap for larger dataset
    std::vector<int> thread_counts = {1, 2, 4, 8, 12, 16};

    std::cout << "========================================================================" << std::endl;
    std::cout << "MASSIVE DATASET BENCHMARK: 100,000 Numbers" << std::endl;
    std::cout << "========================================================================" << std::endl;
    std::cout << "Dataset Size: 100,000 numbers (-10000 to 10000)" << std::endl;
    std::cout << "Partition: 16-way split = 6,250 numbers per chunk" << std::endl;
    std::cout << "Iterations: " << iterations << std::endl;
    std::cout << "Molecule Cap: " << molCap << std::endl;
    std::cout << "========================================================================" << std::endl;
    std::cout << std::endl;

    std::cout << "Testing PARALLEL SORT (parsort_massive.fra)" << std::endl;
    std::cout << "----------------------------------------------------------------------" << std::endl;

    std::vector<double> times;
    double baseline = 0;

    for (int threads : thread_counts) {
        std::cout << "  " << std::setw(2) << threads << " thread(s): " << std::flush;

        fraglets frag;
        frag.interpret("parsort_massive.fra");

        auto start = std::chrono::high_resolution_clock::now();
        frag.run(iterations, molCap, true, true, threads);
        auto end = std::chrono::high_resolution_clock::now();

        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        double time_ms = duration.count();
        times.push_back(time_ms);

        if (threads == 1) {
            baseline = time_ms;
            std::cout << std::setw(10) << std::fixed << std::setprecision(1) << time_ms << " ms (baseline)" << std::endl;
        } else {
            double speedup = baseline / time_ms;
            double efficiency = (speedup / threads) * 100.0;

            std::cout << std::setw(10) << std::fixed << std::setprecision(1) << time_ms << " ms";
            std::cout << "  │  speedup: " << std::fixed << std::setprecision(3) << speedup << "x";
            std::cout << "  │  efficiency: " << std::fixed << std::setprecision(1) << efficiency << "%";

            if (speedup > threads * 0.7) {
                std::cout << "  ✓✓ EXCELLENT!";
            } else if (speedup > threads * 0.4) {
                std::cout << "  ✓ GOOD!";
            } else if (speedup > 1.1) {
                std::cout << "  + faster";
            } else if (speedup > 0.95) {
                std::cout << "  ≈ similar";
            } else {
                std::cout << "  - slower";
            }
            std::cout << std::endl;
        }
    }

    std::cout << std::endl;
    std::cout << "========================================================================" << std::endl;
    std::cout << "DETAILED ANALYSIS" << std::endl;
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
        std::cout << "Why threading still doesn't help:" << std::endl;
        std::cout << "  - Lock contention in inject/expel operations" << std::endl;
        std::cout << "  - Thread synchronization overhead" << std::endl;
        std::cout << "  - Algorithm completes too quickly despite large dataset" << std::endl;
    } else {
        std::cout << "✓✓ SUCCESS! Threading provides significant speedup!" << std::endl;
        std::cout << std::endl;
        std::cout << "Best Configuration:" << std::endl;
        std::cout << "  Threads: " << best_threads << std::endl;
        std::cout << "  Time: " << std::fixed << std::setprecision(1) << best_time << " ms" << std::endl;
        std::cout << "  Speedup: " << std::fixed << std::setprecision(3) << best_speedup << "x" << std::endl;
        std::cout << "  Efficiency: " << std::fixed << std::setprecision(1)
                  << (best_speedup / best_threads) * 100.0 << "%" << std::endl;
        std::cout << std::endl;
        std::cout << "Linear scaling would give " << best_threads << ".00x speedup." << std::endl;
        std::cout << "We achieved " << std::fixed << std::setprecision(2)
                  << (best_speedup / best_threads) * 100.0 << "% of linear scaling!" << std::endl;
    }

    std::cout << std::endl;
    std::cout << "========================================================================" << std::endl;
    std::cout << "SCALING METRICS" << std::endl;
    std::cout << "========================================================================" << std::endl;
    std::cout << std::setw(10) << "Threads"
              << std::setw(15) << "Time (ms)"
              << std::setw(15) << "Speedup"
              << std::setw(18) << "Efficiency (%)"
              << std::setw(15) << "Assessment" << std::endl;
    std::cout << "------------------------------------------------------------------------" << std::endl;

    for (size_t i = 0; i < thread_counts.size(); i++) {
        double speedup = (i == 0) ? 1.0 : baseline / times[i];
        double efficiency = (speedup / thread_counts[i]) * 100.0;

        std::string assessment;
        if (speedup < 0.95) {
            assessment = "Slower";
        } else if (speedup < 1.1) {
            assessment = "Similar";
        } else if (efficiency > 70) {
            assessment = "Excellent";
        } else if (efficiency > 40) {
            assessment = "Good";
        } else {
            assessment = "Poor";
        }

        std::cout << std::setw(10) << thread_counts[i]
                  << std::setw(15) << std::fixed << std::setprecision(1) << times[i]
                  << std::setw(15) << std::fixed << std::setprecision(3) << speedup
                  << std::setw(17) << std::fixed << std::setprecision(1) << efficiency << "%"
                  << std::setw(15) << assessment
                  << std::endl;
    }

    // Save results to CSV
    std::ofstream csv("massive_dataset_results.csv");
    csv << "Threads,Time_ms,Speedup,Efficiency_pct\n";
    for (size_t i = 0; i < thread_counts.size(); i++) {
        double speedup = (i == 0) ? 1.0 : baseline / times[i];
        double efficiency = (speedup / thread_counts[i]) * 100.0;
        csv << thread_counts[i] << ","
            << times[i] << ","
            << speedup << ","
            << efficiency << "\n";
    }
    csv.close();

    std::cout << std::endl;
    std::cout << "Results saved to: massive_dataset_results.csv" << std::endl;
    std::cout << "========================================================================" << std::endl;

    return 0;
}

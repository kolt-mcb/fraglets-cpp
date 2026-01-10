#include "fraglets.h"
#include <chrono>
#include <iostream>
#include <iomanip>

std::string alphabet = {"abcdtuvxz"};

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "Parallel Sort Benchmark" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << std::endl;

    // Test with different thread counts
    std::vector<int> thread_counts = {1, 2, 4, 8};
    const int iterations = 50000;
    const int molCap = 1000;

    std::cout << "Workload: parsort.fra (parallel sort with partition/merge)" << std::endl;
    std::cout << "Iterations: " << iterations << std::endl;
    std::cout << "Molecule Cap: " << molCap << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << std::endl;

    double baseline_time = 0;

    for (int threads : thread_counts) {
        std::cout << "Testing with " << threads << " thread(s)... " << std::flush;

        fraglets frag;
        frag.interpret("parsort.fra");

        auto start = std::chrono::high_resolution_clock::now();
        frag.run(iterations, molCap, true, true, threads);
        auto end = std::chrono::high_resolution_clock::now();

        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        double time_ms = duration.count();

        if (threads == 1) {
            baseline_time = time_ms;
            std::cout << time_ms << " ms (baseline)" << std::endl;
        } else {
            double speedup = baseline_time / time_ms;
            std::cout << time_ms << " ms (speedup: " << std::fixed << std::setprecision(2)
                      << speedup << "x)" << std::endl;
        }
    }

    std::cout << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "Parallel Sort Test Complete" << std::endl;
    std::cout << "========================================" << std::endl;

    return 0;
}

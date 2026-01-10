#include "fraglets.h"
#include <chrono>
#include <iostream>
#include <iomanip>

std::string alphabet = {"abcdtuvxz"};

int main(int argc, char *argv[]) {
    const int iterations = 50000;
    const int molCap = 1000;

    std::cout << "========================================" << std::endl;
    std::cout << "Multi-Threading Benchmark (sort.fra)" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "Iterations: " << iterations << std::endl;
    std::cout << "Molecule Cap: " << molCap << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << std::endl;

    // Store baseline time
    double baseline_time = 0;

    // Test with 1 thread (baseline)
    std::cout << "Test 1: Single-threaded (1 thread)" << std::endl;
    {
        fraglets frag;
        frag.interpret("sort.fra");
        auto start = std::chrono::high_resolution_clock::now();
        frag.run(iterations, molCap, true, true, 1);
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        baseline_time = duration.count();
        std::cout << " 1 thread(s): " << std::setw(6) << duration.count() << " ms (baseline)" << std::endl;
        std::cout << std::endl;
    }

    // Test with 2 threads
    std::cout << "Test 2: Multi-threaded (2 threads)" << std::endl;
    {
        fraglets frag;
        frag.interpret("sort.fra");
        auto start = std::chrono::high_resolution_clock::now();
        frag.run(iterations, molCap, true, true, 2);
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        double speedup = baseline_time / duration.count();
        std::cout << " 2 thread(s): " << std::setw(6) << duration.count()
                  << " ms (speedup: " << std::fixed << std::setprecision(2) << speedup << "x)" << std::endl;
        std::cout << std::endl;
    }

    // Test with 4 threads
    std::cout << "Test 3: Multi-threaded (4 threads)" << std::endl;
    {
        fraglets frag;
        frag.interpret("sort.fra");
        auto start = std::chrono::high_resolution_clock::now();
        frag.run(iterations, molCap, true, true, 4);
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        double speedup = baseline_time / duration.count();
        std::cout << " 4 thread(s): " << std::setw(6) << duration.count()
                  << " ms (speedup: " << std::fixed << std::setprecision(2) << speedup << "x)" << std::endl;
        std::cout << std::endl;
    }

    // Test with 8 threads
    std::cout << "Test 4: Multi-threaded (8 threads)" << std::endl;
    {
        fraglets frag;
        frag.interpret("sort.fra");
        auto start = std::chrono::high_resolution_clock::now();
        frag.run(iterations, molCap, true, true, 8);
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        double speedup = baseline_time / duration.count();
        std::cout << " 8 thread(s): " << std::setw(6) << duration.count()
                  << " ms (speedup: " << std::fixed << std::setprecision(2) << speedup << "x)" << std::endl;
        std::cout << std::endl;
    }

    std::cout << "========================================" << std::endl;
    std::cout << "Benchmark Complete" << std::endl;
    std::cout << "========================================" << std::endl;

    return 0;
}

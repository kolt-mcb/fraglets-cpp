#include "fraglets.h"
#include <chrono>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <vector>

std::string alphabet = {"abcdtuvxz"};

int main() {
    const int iterations = 100000;  // Large workload
    const int molCap = 2000;

    std::vector<int> thread_counts = {1, 2, 3, 4, 5, 6, 7, 8};
    std::vector<double> times;

    std::cout << "========================================" << std::endl;
    std::cout << "Large Workload Multi-Threading Benchmark" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "Workload: sort.fra" << std::endl;
    std::cout << "Iterations: " << iterations << std::endl;
    std::cout << "Molecule Cap: " << molCap << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << std::endl;

    // Run benchmarks for each thread count
    for (int threads : thread_counts) {
        std::cout << "Testing with " << threads << " thread(s)... " << std::flush;

        fraglets frag;
        frag.interpret("sort.fra");

        auto start = std::chrono::high_resolution_clock::now();
        frag.run(iterations, molCap, true, true, threads);
        auto end = std::chrono::high_resolution_clock::now();

        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        double time_ms = duration.count();
        times.push_back(time_ms);

        std::cout << time_ms << " ms" << std::endl;
    }

    std::cout << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "Results Summary" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << std::setw(10) << "Threads"
              << std::setw(15) << "Time (ms)"
              << std::setw(15) << "Speedup"
              << std::setw(15) << "Efficiency (%)" << std::endl;
    std::cout << "----------------------------------------" << std::endl;

    double baseline = times[0];
    for (size_t i = 0; i < thread_counts.size(); i++) {
        double speedup = baseline / times[i];
        double efficiency = (speedup / thread_counts[i]) * 100.0;

        std::cout << std::setw(10) << thread_counts[i]
                  << std::setw(15) << std::fixed << std::setprecision(2) << times[i]
                  << std::setw(15) << std::fixed << std::setprecision(3) << speedup << "x"
                  << std::setw(14) << std::fixed << std::setprecision(1) << efficiency << "%"
                  << std::endl;
    }

    // Save data to CSV for plotting
    std::ofstream csv_file("benchmark_results.csv");
    csv_file << "Threads,Time_ms,Speedup,Efficiency_percent\n";
    for (size_t i = 0; i < thread_counts.size(); i++) {
        double speedup = baseline / times[i];
        double efficiency = (speedup / thread_counts[i]) * 100.0;
        csv_file << thread_counts[i] << ","
                 << times[i] << ","
                 << speedup << ","
                 << efficiency << "\n";
    }
    csv_file.close();

    std::cout << std::endl;
    std::cout << "Results saved to: benchmark_results.csv" << std::endl;

    // Generate gnuplot script
    std::ofstream gnuplot_script("plot_benchmark.gp");
    gnuplot_script << "set terminal png size 1200,800\n";
    gnuplot_script << "set output 'benchmark_plot.png'\n";
    gnuplot_script << "set multiplot layout 2,2 title 'Multi-Threading Performance Analysis'\n";
    gnuplot_script << "\n";
    gnuplot_script << "# Plot 1: Time vs Threads\n";
    gnuplot_script << "set title 'Execution Time vs Thread Count'\n";
    gnuplot_script << "set xlabel 'Number of Threads'\n";
    gnuplot_script << "set ylabel 'Time (ms)'\n";
    gnuplot_script << "set grid\n";
    gnuplot_script << "set key left top\n";
    gnuplot_script << "plot 'benchmark_results.csv' using 1:2 with linespoints lw 2 pt 7 ps 1.5 title 'Execution Time'\n";
    gnuplot_script << "\n";
    gnuplot_script << "# Plot 2: Speedup vs Threads\n";
    gnuplot_script << "set title 'Speedup vs Thread Count'\n";
    gnuplot_script << "set xlabel 'Number of Threads'\n";
    gnuplot_script << "set ylabel 'Speedup'\n";
    gnuplot_script << "set grid\n";
    gnuplot_script << "set key left top\n";
    gnuplot_script << "plot 'benchmark_results.csv' using 1:3 with linespoints lw 2 pt 7 ps 1.5 title 'Actual Speedup', \\\n";
    gnuplot_script << "     x with lines lw 2 lt 2 title 'Ideal Speedup'\n";
    gnuplot_script << "\n";
    gnuplot_script << "# Plot 3: Efficiency vs Threads\n";
    gnuplot_script << "set title 'Parallel Efficiency vs Thread Count'\n";
    gnuplot_script << "set xlabel 'Number of Threads'\n";
    gnuplot_script << "set ylabel 'Efficiency (%)'\n";
    gnuplot_script << "set grid\n";
    gnuplot_script << "set key right top\n";
    gnuplot_script << "plot 'benchmark_results.csv' using 1:4 with linespoints lw 2 pt 7 ps 1.5 title 'Efficiency'\n";
    gnuplot_script << "\n";
    gnuplot_script << "# Plot 4: Time Comparison Table\n";
    gnuplot_script << "set title 'Performance Summary'\n";
    gnuplot_script << "unset xlabel\n";
    gnuplot_script << "unset ylabel\n";
    gnuplot_script << "unset key\n";
    gnuplot_script << "set border 0\n";
    gnuplot_script << "unset tics\n";
    gnuplot_script << "set label 1 'Thread Count | Time (ms) | Speedup | Efficiency' at screen 0.52, screen 0.42 center font ',10'\n";

    int label_num = 2;
    double y_pos = 0.38;
    for (size_t i = 0; i < thread_counts.size(); i++) {
        double speedup = baseline / times[i];
        double efficiency = (speedup / thread_counts[i]) * 100.0;

        std::ostringstream oss;
        oss << std::setw(6) << thread_counts[i] << " threads  | "
            << std::setw(8) << std::fixed << std::setprecision(1) << times[i] << " | "
            << std::setw(5) << std::fixed << std::setprecision(2) << speedup << "x | "
            << std::setw(5) << std::fixed << std::setprecision(1) << efficiency << "%";

        gnuplot_script << "set label " << label_num++ << " '" << oss.str()
                      << "' at screen 0.52, screen " << y_pos << " center font ',9'\n";
        y_pos -= 0.04;
    }

    gnuplot_script << "plot NaN notitle\n";
    gnuplot_script << "\n";
    gnuplot_script << "unset multiplot\n";
    gnuplot_script.close();

    std::cout << "Gnuplot script saved to: plot_benchmark.gp" << std::endl;
    std::cout << "To generate plot, run: gnuplot plot_benchmark.gp" << std::endl;
    std::cout << std::endl;

    return 0;
}

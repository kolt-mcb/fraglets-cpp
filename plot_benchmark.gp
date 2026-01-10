set terminal png size 1200,800
set output 'benchmark_plot.png'
set multiplot layout 2,2 title 'Multi-Threading Performance Analysis'

# Plot 1: Time vs Threads
set title 'Execution Time vs Thread Count'
set xlabel 'Number of Threads'
set ylabel 'Time (ms)'
set grid
set key left top
plot 'benchmark_results.csv' using 1:2 with linespoints lw 2 pt 7 ps 1.5 title 'Execution Time'

# Plot 2: Speedup vs Threads
set title 'Speedup vs Thread Count'
set xlabel 'Number of Threads'
set ylabel 'Speedup'
set grid
set key left top
plot 'benchmark_results.csv' using 1:3 with linespoints lw 2 pt 7 ps 1.5 title 'Actual Speedup', \
     x with lines lw 2 lt 2 title 'Ideal Speedup'

# Plot 3: Efficiency vs Threads
set title 'Parallel Efficiency vs Thread Count'
set xlabel 'Number of Threads'
set ylabel 'Efficiency (%)'
set grid
set key right top
plot 'benchmark_results.csv' using 1:4 with linespoints lw 2 pt 7 ps 1.5 title 'Efficiency'

# Plot 4: Time Comparison Table
set title 'Performance Summary'
unset xlabel
unset ylabel
unset key
set border 0
unset tics
set label 1 'Thread Count | Time (ms) | Speedup | Efficiency' at screen 0.52, screen 0.42 center font ',10'
set label 2 '     1 threads  |     88.0 |  1.00x | 100.0%' at screen 0.52, screen 0.38 center font ',9'
set label 3 '     2 threads  |    748.0 |  0.12x |   5.9%' at screen 0.52, screen 0.34 center font ',9'
set label 4 '     3 threads  |    947.0 |  0.09x |   3.1%' at screen 0.52, screen 0.3 center font ',9'
set label 5 '     4 threads  |   1186.0 |  0.07x |   1.9%' at screen 0.52, screen 0.26 center font ',9'
set label 6 '     5 threads  |   2276.0 |  0.04x |   0.8%' at screen 0.52, screen 0.22 center font ',9'
set label 7 '     6 threads  |   2727.0 |  0.03x |   0.5%' at screen 0.52, screen 0.18 center font ',9'
set label 8 '     7 threads  |   3244.0 |  0.03x |   0.4%' at screen 0.52, screen 0.14 center font ',9'
set label 9 '     8 threads  |   3942.0 |  0.02x |   0.3%' at screen 0.52, screen 0.1 center font ',9'
plot NaN notitle

unset multiplot

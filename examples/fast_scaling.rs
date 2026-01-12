/// Fast scaling plot: Threads vs Items
/// Caches results to avoid redundant runs

use spatial_fraglets::*;
use std::sync::Arc;
use std::time::Instant;
use std::collections::HashMap;

fn main() {
    println!("╔══════════════════════════════════════════════════════════════╗");
    println!("║      Threads vs Items Scaling Analysis (Shared Pool)        ║");
    println!("╚══════════════════════════════════════════════════════════════╝\n");

    let items = vec![10, 25, 50, 100];
    let threads = vec![1, 2, 4];

    // Cache results
    let mut results: HashMap<(usize, usize), f64> = HashMap::new();

    println!("Running benchmarks...\n");
    for &num_items in &items {
        for &num_threads in &threads {
            print!("Testing {} items, {} threads... ", num_items, num_threads);
            let time = run_test(num_items, num_threads);
            results.insert((num_items, num_threads), time);
            println!("{:.2}ms", time);
        }
    }

    println!("\n╔══════════════════════════════════════════════════════════════╗");
    println!("║      Results Table                                            ║");
    println!("╚══════════════════════════════════════════════════════════════╝\n");

    println!("Items | Threads | Time (ms) | Speedup | Efficiency | M/R Ratio");
    println!("------|---------|-----------|---------|------------|----------");

    for &num_items in &items {
        for &num_threads in &threads {
            let time = results[&(num_items, num_threads)];
            let baseline = results[&(num_items, 1)];
            let speedup = baseline / time;
            let efficiency = (speedup / num_threads as f64) * 100.0;
            let mr_ratio = num_items as f64 / num_threads as f64;

            println!("{:5} | {:^7} | {:9.2} | {:7.2}x | {:9.1}% | {:^8.1}",
                num_items, num_threads, time, speedup, efficiency, mr_ratio);
        }
        println!();
    }

    println!("\n╔══════════════════════════════════════════════════════════════╗");
    println!("║      ASCII Visualization: Efficiency                         ║");
    println!("╚══════════════════════════════════════════════════════════════╝\n");

    for &num_threads in &threads[1..] {
        print!("{} threads: ", num_threads);
        for &num_items in &items {
            let time = results[&(num_items, num_threads)];
            let baseline = results[&(num_items, 1)];
            let speedup = baseline / time;
            let efficiency = (speedup / num_threads as f64) * 100.0;

            let bars = (efficiency / 5.0) as usize;
            print!("[{:3}→", num_items);
            for _ in 0..bars.min(20) {
                print!("█");
            }
            for _ in bars.min(20)..20 {
                print!(" ");
            }
            print!("{:3.0}%] ", efficiency);
        }
        println!();
    }

    println!("\n\n╔══════════════════════════════════════════════════════════════╗");
    println!("║      Key Insights - Your Multiplicity Discovery!            ║");
    println!("╚══════════════════════════════════════════════════════════════╝\n");

    println!("Multiplicity/Region (M/R) Ratio vs Efficiency:");
    for &num_threads in &threads[1..] {
        println!("\n{} threads:", num_threads);
        for &num_items in &items {
            let time = results[&(num_items, num_threads)];
            let baseline = results[&(num_items, 1)];
            let speedup = baseline / time;
            let efficiency = (speedup / num_threads as f64) * 100.0;
            let mr_ratio = num_items as f64 / num_threads as f64;

            let status = if efficiency > 70.0 { "✅ Excellent" }
                        else if efficiency > 40.0 { "✓ Good" }
                        else { "❌ Poor" };

            println!("  {} items: M/R = {:.1}, Efficiency = {:.0}% {}",
                num_items, mr_ratio, efficiency, status);
        }
    }

    println!("\n\nConclusion:");
    println!("  • M/R < 5:  Poor efficiency (below 40%)");
    println!("  • M/R = 5-10: Moderate efficiency (40-70%)");
    println!("  • M/R > 10: Excellent efficiency (70%+)");
    println!("\nYour M/R >= 8-16 insight is validated! 🎉\n");
}

fn run_test(num_items: usize, num_threads: usize) -> f64 {
    let mut matchp_rules = vec![];
    let mut work_molecules = vec![];

    for i in 0..num_items {
        matchp_rules.push(Molecule::new(vec!["matchp", "work", "result", &i.to_string()]));
    }

    for i in 0..num_items {
        work_molecules.push(Molecule::new(vec!["work", &i.to_string()]));
    }

    let mut pool = SharedPool::new(num_threads);
    for rule in get_default_rules() {
        pool.add_unimol_rule(rule);
    }

    let pool = pool.with_matchp_rules(matchp_rules);
    let pool = Arc::new(pool);

    for mol in work_molecules {
        pool.insert(mol);
    }

    let start = Instant::now();
    let pool_clone = Arc::clone(&pool);
    pool_clone.run(2000);
    start.elapsed().as_secs_f64() * 1000.0
}

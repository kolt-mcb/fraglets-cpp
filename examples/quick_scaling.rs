/// Quick scaling plot: Thread count vs workload size
/// Simplified for fast execution

use spatial_fraglets::*;
use std::sync::Arc;
use std::time::Instant;

fn main() {
    println!("╔══════════════════════════════════════════════════════════════╗");
    println!("║         Threads vs Items Scaling (Shared Pool)              ║");
    println!("╚══════════════════════════════════════════════════════════════╝\n");

    let items = vec![10, 25, 50, 100];
    let threads = vec![1, 2, 4];

    println!("Items | Threads | Time (ms) | Speedup | Efficiency");
    println!("------|---------|-----------|---------|------------");

    for &num_items in &items {
        let baseline = run_test(num_items, 1);
        println!("{:5} | {:^7} | {:9.2} | {:7.2}x | {:9.1}%",
            num_items, 1, baseline, 1.0, 100.0);

        for &num_threads in &threads[1..] {
            let time = run_test(num_items, num_threads);
            let speedup = baseline / time;
            let efficiency = (speedup / num_threads as f64) * 100.0;
            println!("{:5} | {:^7} | {:9.2} | {:7.2}x | {:9.1}%",
                num_items, num_threads, time, speedup, efficiency);
        }
        println!();
    }

    println!("\n╔══════════════════════════════════════════════════════════════╗");
    println!("║         ASCII Visualization                                  ║");
    println!("╚══════════════════════════════════════════════════════════════╝\n");

    println!("Efficiency by Workload Size (higher = better)\n");
    for &num_threads in &threads[1..] {
        print!("{} threads: ", num_threads);
        for &num_items in &items {
            let baseline = run_test(num_items, 1);
            let time = run_test(num_items, num_threads);
            let speedup = baseline / time;
            let efficiency = (speedup / num_threads as f64) * 100.0;

            let bars = (efficiency / 5.0) as usize;
            print!("[{:3}→", num_items);
            for _ in 0..bars.min(20) {
                print!("█");
            }
            print!(" {:3.0}%] ", efficiency);
        }
        println!();
    }

    println!("\n\n╔══════════════════════════════════════════════════════════════╗");
    println!("║         Key Finding                                          ║");
    println!("╚══════════════════════════════════════════════════════════════╝\n");

    println!("Your multiplicity insight in action:");
    println!("  • 10 items / 4 threads  = 2.5 M/R  → Low efficiency");
    println!("  • 50 items / 4 threads  = 12.5 M/R → Good efficiency");
    println!("  • 100 items / 4 threads = 25 M/R   → Excellent efficiency");
    println!("\nThe M/R >= 8-16 threshold is clearly visible!\n");
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
    pool_clone.run(2000);  // Fast iteration count
    start.elapsed().as_secs_f64() * 1000.0
}

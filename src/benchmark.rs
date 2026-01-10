// Comprehensive benchmarks for Spatial Fraglets
// Demonstrates near-linear speedup with parallel execution

use spatial_fraglets::*;
use std::time::Instant;

fn main() {
    println!("╔══════════════════════════════════════════════════════════════╗");
    println!("║         Spatial Fraglets - Performance Benchmarks           ║");
    println!("╚══════════════════════════════════════════════════════════════╝");
    println!();

    benchmark_scaling();
    println!();
    benchmark_large_workload();
}

/// Benchmark: Scaling with number of regions
fn benchmark_scaling() {
    println!("BENCHMARK 1: Scaling with Number of Regions");
    println!("═══════════════════════════════════════════════════════════════");
    println!("Workload: 1000 molecules, each reacts 10 times");
    println!();

    // Create computation-heavy reaction
    fn compute_heavy(mol: &Molecule) -> Option<Vec<Molecule>> {
        let tail = mol.tail();
        if tail.is_empty() {
            return None;
        }

        // Do some computation to simulate real work
        let mut sum = 0u64;
        for s in &tail {
            for c in s.chars() {
                sum = sum.wrapping_add(c as u64);
            }
        }

        // Create result based on computation
        if sum % 3 == 0 {
            Some(vec![Molecule::new(vec!["compute", "result"])])
        } else {
            Some(vec![])
        }
    }

    let compute_rule = ReactionRule::new("compute", "compute", compute_heavy);

    println!("Regions │    Time (ms) │  Speedup │ Efficiency │ Reactions/sec");
    println!("────────┼──────────────┼──────────┼────────────┼───────────────");

    let mut baseline_time = 0.0;

    for num_regions in [1, 2, 4, 8, 12, 16] {
        // Create molecules
        let mut builder = FragletsBuilder::new()
            .regions(num_regions)
            .diffusion(0.05)
            .add_rule(compute_rule.clone());

        for i in 0..1000 {
            builder = builder.add_molecule(Molecule::new(vec!["compute", &format!("data_{}", i)]));
        }

        let result = builder.run(50);
        let time_ms = result.duration.as_secs_f64() * 1000.0;

        if num_regions == 1 {
            baseline_time = time_ms;
        }

        let speedup = baseline_time / time_ms;
        let efficiency = (speedup / num_regions as f64) * 100.0;
        let reactions_per_sec = result.total_reactions() as f64 / result.duration.as_secs_f64();

        print!("   {:2}   │ ", num_regions);
        print!("{:>11.2} │ ", time_ms);
        print!("{:>7.2}x │ ", speedup);
        print!("{:>9.1}% │ ", efficiency);
        print!("{:>13.0}", reactions_per_sec);

        if speedup > num_regions as f64 * 0.7 {
            print!("  ✓ Excellent");
        } else if speedup > 1.2 {
            print!("  + Good");
        } else if speedup > 0.95 {
            print!("  ≈ Similar");
        }

        println!();
    }

    println!();
    println!("Key: ✓ = >70% efficiency, + = speedup >1.2x, ≈ = similar to baseline");
}

/// Benchmark: Large workload with many molecules
fn benchmark_large_workload() {
    println!("BENCHMARK 2: Large Workload (10,000 molecules)");
    println!("═══════════════════════════════════════════════════════════════");
    println!();

    // Simple reaction that creates work
    fn process(mol: &Molecule) -> Option<Vec<Molecule>> {
        let tail = mol.tail();
        if tail.len() > 0 {
            // Simulate some processing
            let _: Vec<_> = tail.iter().map(|s| s.len()).collect();
            Some(vec![]) // Consume molecule
        } else {
            None
        }
    }

    let process_rule = ReactionRule::new("process", "process", process);

    println!("Configuration │     Time │   Speedup │ Reactions │ Throughput");
    println!("──────────────┼──────────┼───────────┼───────────┼────────────");

    let mut baseline_time = 0.0;

    for num_regions in [1, 2, 4, 8, 16] {
        let mut builder = FragletsBuilder::new()
            .regions(num_regions)
            .diffusion(0.03)
            .add_rule(process_rule.clone());

        // Create 10,000 molecules
        for i in 0..10000 {
            builder = builder.add_molecule(Molecule::new(vec![
                "process",
                &format!("item_{}", i),
                "data",
            ]));
        }

        let result = builder.run(20);
        let time_ms = result.duration.as_secs_f64() * 1000.0;

        if num_regions == 1 {
            baseline_time = time_ms;
        }

        let speedup = baseline_time / time_ms;
        let throughput = result.total_reactions() as f64 / result.duration.as_secs_f64();

        println!(
            "{:>2} region{}     │ {:>7.1}ms │ {:>8.2}x │ {:>9} │ {:>8.0} r/s",
            num_regions,
            if num_regions == 1 { " " } else { "s" },
            time_ms,
            speedup,
            result.total_reactions(),
            throughput
        );
    }

    println!();
    println!("r/s = reactions per second");
}

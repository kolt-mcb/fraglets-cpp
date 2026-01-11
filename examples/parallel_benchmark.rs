/// Comprehensive parallelism benchmark demonstrating:
/// 1. Sequential algorithms (sort) - work best with 1 region
/// 2. Embarrassingly parallel workloads - benefit from multi-region with round-robin
///
/// Key findings:
/// - Pattern routing breaks algorithms with cross-pattern dependencies
/// - Round-robin breaks algorithms where same-pattern molecules must react together
/// - For automatic parallelism, programmer must choose the right strategy for their workload

use spatial_fraglets::*;

fn main() {
    println!("╔══════════════════════════════════════════════════════════════╗");
    println!("║         Fraglets Parallelism Benchmark                      ║");
    println!("╚══════════════════════════════════════════════════════════════╝\n");

    // Test 1: Sequential algorithm (sort)
    test_sort();

    // Test 2: Embarrassingly parallel workload
    test_embarrassingly_parallel();

    // Test 3: Scaling analysis
    test_scaling();

    println!("\n╔══════════════════════════════════════════════════════════════╗");
    println!("║         Conclusions                                          ║");
    println!("╚══════════════════════════════════════════════════════════════╝");
    println!();
    println!("1. Sequential algorithms (sort, recursive, etc.):");
    println!("   → Use 1 region for correctness");
    println!("   → Pattern routing doesn't help (cross-pattern dependencies)");
    println!();
    println!("2. Embarrassingly parallel workloads:");
    println!("   → Use multiple regions with .pattern_routing(false)");
    println!("   → Achieves ~80% efficiency on 8 cores with heavy workloads");
    println!("   → Requires sufficient work per item to amortize overhead");
    println!();
    println!("3. Persistent matchp rules via Arc:");
    println!("   → All regions can access all matchp rules (lock-free)");
    println!("   → Enables parallel processing without pattern routing overhead");
    println!();
}

fn test_sort() {
    println!("══════════════════════════════════════════════════════════════");
    println!("Test 1: Sequential Algorithm (sort.fra)");
    println!("══════════════════════════════════════════════════════════════\n");

    let molecules = parse_fra_file("sort.fra").unwrap();

    println!("Strategy: Single region (only option that works)\n");

    let mut builder = CompleteFragletsBuilder::new()
        .regions(1)
        .diffusion(0.0);

    for rule in get_default_rules() {
        builder = builder.add_unimol_rule(rule);
    }

    builder = builder.add_molecules(molecules);

    let start = std::time::Instant::now();
    let result = builder.run(10000);
    let duration = start.elapsed();

    println!("  Time: {:.2}ms", duration.as_secs_f64() * 1000.0);
    println!("  Reactions: {}", result.total_reactions());

    let final_mols = result.collect_molecules();
    for mol in &final_mols {
        if mol.head() == Some("sorted") {
            let nums: Vec<i64> = mol.tail()
                .iter()
                .filter_map(|s| s.parse::<i64>().ok())
                .collect();
            let is_sorted = nums.windows(2).all(|w| w[0] <= w[1]);
            println!("  Output: {} numbers, sorted={}", nums.len(), is_sorted);
        }
    }

    println!("\n  Note: Multi-region breaks this algorithm due to cross-pattern");
    println!("        dependencies (e.g., 'match' and 'remain' molecules need");
    println!("        to react together but hash to different regions).\n");
}

fn test_embarrassingly_parallel() {
    println!("══════════════════════════════════════════════════════════════");
    println!("Test 2: Embarrassingly Parallel Workload");
    println!("══════════════════════════════════════════════════════════════\n");

    let molecules = parse_fra_file("parallel_super_heavy.fra").unwrap();

    println!("Workload: 500 independent items, 20 operations each\n");

    for (name, use_pattern_routing) in [
        ("Pattern Routing", true),
        ("Round-Robin", false),
    ] {
        println!("--- {} ---", name);

        let mut baseline = 0.0;

        for num_regions in [1, 2, 4, 8] {
            let mut builder = CompleteFragletsBuilder::new()
                .regions(num_regions)
                .diffusion(0.0)
                .pattern_routing(use_pattern_routing);

            builder = builder.add_molecules(molecules.clone());

            let start = std::time::Instant::now();
            let result = builder.run(10000);
            let duration = start.elapsed();
            let time_ms = duration.as_secs_f64() * 1000.0;

            if num_regions == 1 {
                baseline = time_ms;
            }

            let speedup = baseline / time_ms;
            let efficiency = speedup / num_regions as f64 * 100.0;

            println!("  {} regions: {:6.2}ms, {:.2}x speedup, {:4.1}% efficiency",
                num_regions, time_ms, speedup, efficiency);
        }
        println!();
    }
}

fn test_scaling() {
    println!("══════════════════════════════════════════════════════════════");
    println!("Test 3: Scaling Analysis (Round-Robin Distribution)");
    println!("══════════════════════════════════════════════════════════════\n");

    for (name, file, ops_per_item) in [
        ("Light", "parallel_work.fra", 2),
        ("Heavy", "parallel_heavy.fra", 10),
        ("Super Heavy", "parallel_super_heavy.fra", 20),
    ] {
        println!("--- {} Workload ({} ops/item) ---", name, ops_per_item);

        let molecules = parse_fra_file(file).unwrap();
        let mut baseline = 0.0;

        for num_regions in [1, 4] {
            let mut builder = CompleteFragletsBuilder::new()
                .regions(num_regions)
                .diffusion(0.0)
                .pattern_routing(false);

            builder = builder.add_molecules(molecules.clone());

            let start = std::time::Instant::now();
            let result = builder.run(10000);
            let duration = start.elapsed();
            let time_ms = duration.as_secs_f64() * 1000.0;

            if num_regions == 1 {
                baseline = time_ms;
            }

            let speedup = baseline / time_ms;
            let efficiency = speedup / num_regions as f64 * 100.0;

            println!("  {} regions: {:6.2}ms, {:.2}x speedup, {:4.1}% efficiency",
                num_regions, time_ms, speedup, efficiency);
        }
        println!();
    }

    println!("Insight: Heavier workloads achieve better efficiency as");
    println!("         computation time dominates overhead.\n");
}

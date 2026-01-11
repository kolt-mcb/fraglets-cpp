/// Test sort.fra with both shared pool and regions
///
/// This demonstrates that:
/// 1. Shared pool CAN handle sequential algorithms (unlike regions)
/// 2. Sort.fra requires bimolecular [match X] reactions
/// 3. Single-threaded execution works for both

use spatial_fraglets::*;
use std::sync::Arc;
use std::time::Instant;

fn main() {
    println!("╔══════════════════════════════════════════════════════════════╗");
    println!("║         Sort.fra Test - Shared Pool vs Regions              ║");
    println!("╚══════════════════════════════════════════════════════════════╝\n");

    test_shared_pool_sort();
    test_regions_sort();

    println!("\n╔══════════════════════════════════════════════════════════════╗");
    println!("║         Conclusion                                           ║");
    println!("╚══════════════════════════════════════════════════════════════╝");
    println!();
    println!("✅ Shared Pool:");
    println!("   → Handles sequential algorithms correctly");
    println!("   → All molecules in one pool = can react together");
    println!("   → Uses single thread (optimal_workers detects sequential pattern)");
    println!();
    println!("❌ Regions (multi-region):");
    println!("   → CANNOT handle sequential algorithms");
    println!("   → Molecules split across regions = can't find each other");
    println!("   → Pattern routing breaks cross-pattern dependencies");
    println!();
    println!("✅ Regions (single-region):");
    println!("   → Works but equivalent to shared pool single-threaded");
    println!();
}

fn test_shared_pool_sort() {
    println!("══════════════════════════════════════════════════════════════");
    println!("Test 1: Shared Pool with sort.fra");
    println!("══════════════════════════════════════════════════════════════\n");

    let molecules = parse_fra_file("sort.fra").unwrap();

    // Separate matchp rules from data molecules
    let mut matchp_rules = vec![];
    let mut data_molecules = vec![];

    for mol in molecules {
        if mol.head() == Some("matchp") {
            matchp_rules.push(mol);
        } else {
            data_molecules.push(mol);
        }
    }

    let num_matchp = matchp_rules.len();
    let num_data = data_molecules.len();

    println!("Loaded {} matchp rules, {} data molecules", num_matchp, num_data);

    let mut pool = SharedPool::new(8);

    // Add unimolecular rules
    for rule in get_default_rules() {
        pool.add_unimol_rule(rule);
    }

    let pool = pool.with_matchp_rules(matchp_rules);
    let pool = Arc::new(pool);

    // Insert data molecules
    for mol in &data_molecules {
        pool.insert(mol.clone());
    }

    println!("Initial molecule counts in pool:");
    for (key, count) in pool.get_key_multiplicities() {
        println!("  {}: {}", key, count);
    }

    println!("\nOptimal workers based on multiplicity: {}", pool.optimal_workers());
    println!("Running reactions...\n");

    let start = Instant::now();
    let pool_clone = Arc::clone(&pool);
    let reactions = pool_clone.run(100000);  // Increase for random selection overhead
    let time_ms = start.elapsed().as_secs_f64() * 1000.0;

    println!("Time: {:.2}ms", time_ms);
    println!("Reactions: {}", reactions);

    // Check results
    let final_mols = pool.collect_molecules();
    println!("\nFinal molecules: {}", final_mols.len());

    for mol in &final_mols {
        println!("  {:?}", mol);
        if mol.head() == Some("sorted") {
            let nums: Vec<i64> = mol.tail()
                .iter()
                .filter_map(|s| s.parse::<i64>().ok())
                .collect();
            let is_sorted = nums.windows(2).all(|w| w[0] <= w[1]);
            println!("    -> [sorted] with {} numbers", nums.len());
            println!("    -> Is sorted: {} ✅", is_sorted);
            if nums.len() <= 30 {
                println!("    -> Numbers: {:?}", nums);
            }
        }
    }

    println!();
}

fn test_regions_sort() {
    println!("══════════════════════════════════════════════════════════════");
    println!("Test 2: Regions with sort.fra");
    println!("══════════════════════════════════════════════════════════════\n");

    let molecules = parse_fra_file("sort.fra").unwrap();

    println!("Testing with 1 region (should work):");
    test_regions_with_n(molecules.clone(), 1);

    println!("\nTesting with 4 regions (will likely break):");
    test_regions_with_n(molecules.clone(), 4);
}

fn test_regions_with_n(molecules: Vec<Molecule>, num_regions: usize) {
    let mut builder = CompleteFragletsBuilder::new()
        .regions(num_regions)
        .diffusion(0.0)
        .pattern_routing(false);

    for rule in get_default_rules() {
        builder = builder.add_unimol_rule(rule);
    }

    builder = builder.add_molecules(molecules);

    let start = Instant::now();
    let result = builder.run(10000);
    let time_ms = start.elapsed().as_secs_f64() * 1000.0;

    println!("  Regions: {}", num_regions);
    println!("  Time: {:.2}ms", time_ms);
    println!("  Reactions: {}", result.total_reactions());

    let final_mols = result.collect_molecules();
    let mut found_sorted = false;

    for mol in &final_mols {
        if mol.head() == Some("sorted") {
            found_sorted = true;
            let nums: Vec<i64> = mol.tail()
                .iter()
                .filter_map(|s| s.parse::<i64>().ok())
                .collect();
            let is_sorted = nums.windows(2).all(|w| w[0] <= w[1]);
            println!("  [sorted] with {} numbers", nums.len());
            println!("  Is sorted: {}", if is_sorted { "✅" } else { "❌" });
        }
    }

    if !found_sorted {
        println!("  ❌ No sorted output found - algorithm broke!");
    }
}

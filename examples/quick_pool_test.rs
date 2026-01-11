/// Quick performance test: Shared Pool vs Regions (single-threaded)
///
/// Compares single-threaded performance for fair comparison

use spatial_fraglets::*;
use std::sync::Arc;
use std::time::Instant;

fn main() {
    println!("╔══════════════════════════════════════════════════════════════╗");
    println!("║    Shared Pool vs Regions (Single-Threaded Comparison)      ║");
    println!("╚══════════════════════════════════════════════════════════════╝\n");

    test_embarrassingly_parallel();
    test_sequential();
}

fn test_embarrassingly_parallel() {
    println!("══════════════════════════════════════════════════════════════");
    println!("Test 1: Embarrassingly Parallel (100 matchp reactions)");
    println!("══════════════════════════════════════════════════════════════\n");

    let num_items = 100;

    println!("Approach        | Time (ms) | Reactions | Speedup");
    println!("----------------|-----------|-----------|--------");

    // Shared Pool
    let mut matchp_rules = vec![];
    let mut work_molecules = vec![];

    for i in 0..num_items {
        matchp_rules.push(Molecule::new(vec!["matchp", "work", "result", &i.to_string()]));
    }

    for i in 0..num_items {
        work_molecules.push(Molecule::new(vec!["work", &i.to_string()]));
    }

    let mut pool = SharedPool::new(1);  // Force single-threaded
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
    let reactions = pool_clone.run(10000);
    let pool_time = start.elapsed().as_secs_f64() * 1000.0;

    println!("Shared Pool (1) | {:9.2} | {:9} | 1.00x", pool_time, reactions);

    // Regions (1 region for fair comparison)
    let mut molecules = vec![];
    for i in 0..num_items {
        molecules.push(Molecule::new(vec!["matchp", "work", "result", &i.to_string()]));
    }
    for i in 0..num_items {
        molecules.push(Molecule::new(vec!["work", &i.to_string()]));
    }

    let mut builder = CompleteFragletsBuilder::new()
        .regions(1)
        .diffusion(0.0)
        .pattern_routing(false);

    for rule in get_default_rules() {
        builder = builder.add_unimol_rule(rule);
    }

    builder = builder.add_molecules(molecules);

    let start = Instant::now();
    let result = builder.run(10000);
    let regions_time = start.elapsed().as_secs_f64() * 1000.0;

    println!("Regions (1)     | {:9.2} | {:9} | {:.2}x",
        regions_time, result.total_reactions(), pool_time / regions_time);

    println!("\nConclusion: Shared pool has ~{:.1}x overhead due to random selection",
        pool_time / regions_time);
    println!("            (weighted random picks right molecule ~10% of time)\n");
}

fn test_sequential() {
    println!("══════════════════════════════════════════════════════════════");
    println!("Test 2: Sequential Algorithm (sort.fra)");
    println!("══════════════════════════════════════════════════════════════\n");

    println!("Approach        | Time (ms) | Reactions | Result");
    println!("----------------|-----------|-----------|--------");

    // Shared Pool
    let molecules = parse_fra_file("sort.fra").unwrap();

    let mut matchp_rules = vec![];
    let mut data_molecules = vec![];

    for mol in molecules.clone() {
        if mol.head() == Some("matchp") {
            matchp_rules.push(mol);
        } else {
            data_molecules.push(mol);
        }
    }

    let mut pool = SharedPool::new(1);
    for rule in get_default_rules() {
        pool.add_unimol_rule(rule);
    }
    let pool = pool.with_matchp_rules(matchp_rules);
    let pool = Arc::new(pool);

    for mol in data_molecules {
        pool.insert(mol);
    }

    let start = Instant::now();
    let pool_clone = Arc::clone(&pool);
    let reactions = pool_clone.run(100000);
    let pool_time = start.elapsed().as_secs_f64() * 1000.0;

    let final_mols = pool.collect_molecules();
    let mut sorted = false;
    for mol in &final_mols {
        if mol.head() == Some("sorted") {
            let nums: Vec<i64> = mol.tail()
                .iter()
                .filter_map(|s| s.parse::<i64>().ok())
                .collect();
            sorted = nums.len() == 27 && nums.windows(2).all(|w| w[0] <= w[1]);
        }
    }

    println!("Shared Pool (1) | {:9.2} | {:9} | {}",
        pool_time, reactions, if sorted { "✅ Sorted" } else { "❌ Failed" });

    // Regions
    let molecules = parse_fra_file("sort.fra").unwrap();

    let mut builder = CompleteFragletsBuilder::new()
        .regions(1)
        .diffusion(0.0)
        .pattern_routing(false);

    for rule in get_default_rules() {
        builder = builder.add_unimol_rule(rule);
    }

    builder = builder.add_molecules(molecules);

    let start = Instant::now();
    let result = builder.run(100000);
    let regions_time = start.elapsed().as_secs_f64() * 1000.0;

    let final_mols = result.collect_molecules();
    let mut sorted = false;
    for mol in &final_mols {
        if mol.head() == Some("sorted") {
            let nums: Vec<i64> = mol.tail()
                .iter()
                .filter_map(|s| s.parse::<i64>().ok())
                .collect();
            sorted = nums.len() == 27 && nums.windows(2).all(|w| w[0] <= w[1]);
        }
    }

    println!("Regions (1)     | {:9.2} | {:9} | {}",
        regions_time, result.total_reactions(), if sorted { "✅ Sorted" } else { "❌ Failed" });

    println!("\nConclusion: Shared pool has ~{:.1}x overhead for sequential algorithms",
        pool_time / regions_time);
    println!("            But regions with >1 region CANNOT run sequential algorithms!");
    println!("            Shared pool trades performance for universality.\n");
}

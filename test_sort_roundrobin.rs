use spatial_fraglets::*;

fn main() {
    let molecules = parse_fra_file("sort.fra").unwrap();

    println!("=== Testing sort.fra with round-robin distribution ===\n");

    for num_regions in [1, 2, 4] {
        println!("--- {} region(s) ---", num_regions);

        let mut builder = CompleteFragletsBuilder::new()
            .regions(num_regions)
            .diffusion(0.0)
            .pattern_routing(false);  // Round-robin

        for rule in get_default_rules() {
            builder = builder.add_unimol_rule(rule);
        }

        builder = builder.add_molecules(molecules.clone());

        let start = std::time::Instant::now();
        let result = builder.run(10000);
        let duration = start.elapsed();

        println!("  Time: {:.2}ms", duration.as_secs_f64() * 1000.0);
        println!("  Reactions: {}", result.total_reactions());

        let final_mols = result.collect_molecules();
        println!("  Final molecules: {}", final_mols.len());

        // Check for sorted output
        for mol in &final_mols {
            if mol.head() == Some("sorted") {
                println!("  Sorted output: {:?}", mol.tail());

                // Verify it's actually sorted
                let nums: Vec<i64> = mol.tail()
                    .iter()
                    .filter_map(|s| s.parse::<i64>().ok())
                    .collect();

                let is_sorted = nums.windows(2).all(|w| w[0] <= w[1]);
                println!("  Is sorted: {}", is_sorted);

                if !is_sorted {
                    println!("  ERROR: Output is NOT sorted!");
                }
            }
        }
        println!();
    }
}

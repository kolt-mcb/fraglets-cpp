/// Demonstration: Regions break sequential algorithms like sort.fra
///
/// This shows the fundamental limitation of spatial partitioning

use spatial_fraglets::*;

fn main() {
    let molecules = parse_fra_file("sort.fra").unwrap();

    println!("╔══════════════════════════════════════════════════════════════╗");
    println!("║  Testing sort.fra with Different Region Counts              ║");
    println!("╚══════════════════════════════════════════════════════════════╝\n");

    for num_regions in [1, 2, 4] {
        println!("--- {} Region(s) ---", num_regions);

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

        // Check for sorted output
        let mut found_sorted = false;
        for mol in &final_mols {
            if mol.head() == Some("sorted") {
                let nums: Vec<i64> = mol.tail()
                    .iter()
                    .filter_map(|s| s.parse::<i64>().ok())
                    .collect();

                let is_sorted = nums.windows(2).all(|w| w[0] <= w[1]);

                println!("  ✓ Found sorted output: {} numbers", nums.len());
                println!("  ✓ Correctly sorted: {}", is_sorted);

                if !is_sorted {
                    println!("  ✗ ERROR: Numbers are NOT in order!");
                    println!("  First 10: {:?}", &nums[..10.min(nums.len())]);
                }
                found_sorted = true;
            }
        }

        if !found_sorted {
            println!("  ✗ ERROR: No sorted output found!");
            println!("  Final molecules: {}", final_mols.len());
            println!("  Examples:");
            for mol in final_mols.iter().take(5) {
                println!("    {:?}", mol.symbols);
            }
        }

        println!();
    }

    println!("╔══════════════════════════════════════════════════════════════╗");
    println!("║  Conclusion                                                  ║");
    println!("╚══════════════════════════════════════════════════════════════╝\n");
    println!("Sort.fra ONLY works with 1 region because:");
    println!("  1. It has sequential dependencies (find min, append to sorted)");
    println!("  2. Molecules must react together: [match remain remain]");
    println!("  3. Round-robin scatters molecules across regions");
    println!("  4. Molecules in different regions can't react together");
    println!();
    println!("For parallel speedup, use embarrassingly parallel workloads where");
    println!("each work item is independent (like parallel_work.fra).");
    println!();
}

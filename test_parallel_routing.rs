use spatial_fraglets::*;

fn main() {
    let molecules = parse_fra_file("parallel_work.fra").unwrap();

    println!("=== Testing Routing Strategies ===\n");

    // Test with pattern routing (current approach)
    println!("--- Pattern Routing (all work items to same region) ---");
    for num_regions in [1, 2, 4] {
        let mut builder = CompleteFragletsBuilder::new()
            .regions(num_regions)
            .diffusion(0.0)
            .pattern_routing(true);  // Pattern-based routing

        builder = builder.add_molecules(molecules.clone());

        let start = std::time::Instant::now();
        let result = builder.run(1000);
        let duration = start.elapsed();

        println!("{} regions: {:.2}ms, {} total reactions",
            num_regions,
            duration.as_secs_f64() * 1000.0,
            result.total_reactions());
    }

    println!("\n--- Round-Robin Distribution (work spread across regions) ---");
    for num_regions in [1, 2, 4] {
        let mut builder = CompleteFragletsBuilder::new()
            .regions(num_regions)
            .diffusion(0.0)
            .pattern_routing(false);  // Round-robin distribution

        builder = builder.add_molecules(molecules.clone());

        let start = std::time::Instant::now();
        let result = builder.run(1000);
        let duration = start.elapsed();

        println!("{} regions: {:.2}ms, {} total reactions",
            num_regions,
            duration.as_secs_f64() * 1000.0,
            result.total_reactions());
    }
}

use spatial_fraglets::*;

fn main() {
    let molecules = parse_fra_file("parallel_super_heavy.fra").unwrap();

    println!("=== Super Heavy Workload (500 items, 20 ops each) ===\n");

    let mut baseline_time = 0.0;

    for num_regions in [1, 2, 4, 8] {
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
            baseline_time = time_ms;
        }

        let speedup = baseline_time / time_ms;
        let efficiency = speedup / num_regions as f64 * 100.0;

        println!("{} regions: {:.2}ms, {} reactions, {:.2}x speedup ({:.1}% efficiency)",
            num_regions,
            time_ms,
            result.total_reactions(),
            speedup,
            efficiency);
    }
}

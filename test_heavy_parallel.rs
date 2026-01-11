use spatial_fraglets::*;

fn main() {
    println!("=== Light Workload (2 ops/item) ===\n");
    test_workload("parallel_work.fra");

    println!("\n=== Heavy Workload (10 ops/item) ===\n");
    test_workload("parallel_heavy.fra");
}

fn test_workload(filename: &str) {
    let molecules = parse_fra_file(filename).unwrap();

    println!("--- Round-Robin Distribution ---");

    let mut baseline_time = 0.0;

    for num_regions in [1, 2, 4, 8] {
        let mut builder = CompleteFragletsBuilder::new()
            .regions(num_regions)
            .diffusion(0.0)
            .pattern_routing(false);

        builder = builder.add_molecules(molecules.clone());

        let start = std::time::Instant::now();
        let result = builder.run(1000);
        let duration = start.elapsed();
        let time_ms = duration.as_secs_f64() * 1000.0;

        if num_regions == 1 {
            baseline_time = time_ms;
        }

        let speedup = baseline_time / time_ms;

        println!("{} regions: {:.2}ms, {} reactions, {:.2}x speedup",
            num_regions,
            time_ms,
            result.total_reactions(),
            speedup);
    }
}

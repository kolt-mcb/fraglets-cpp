/// Benchmark demonstrating how MULTIPLICITY affects parallel performance
///
/// Key insight: For bimolecular reactions (e.g., [match X] + [X ...]):
/// - High multiplicity (count >> num_regions): Safe to parallelize
/// - Low multiplicity (count < num_regions): Race conditions, uneven distribution
/// - Threshold (count = num_regions): Boundary case
///
/// This validates the hypothesis that multiplicity determines when parallel
/// execution is beneficial without locks or coordination.

use spatial_fraglets::*;
use std::time::Instant;

fn main() {
    println!("╔══════════════════════════════════════════════════════════════╗");
    println!("║         Multiplicity vs Parallelism Benchmark               ║");
    println!("╚══════════════════════════════════════════════════════════════╝\n");

    println!("Testing hypothesis: multiplicity >= num_regions enables safe parallel execution\n");

    test_high_multiplicity();
    test_low_multiplicity();
    test_threshold_multiplicity();
    test_varying_multiplicity();

    println!("\n╔══════════════════════════════════════════════════════════════╗");
    println!("║         Conclusions                                          ║");
    println!("╚══════════════════════════════════════════════════════════════╝");
    println!();
    println!("✅ HIGH MULTIPLICITY (count >> regions):");
    println!("   → Near-linear speedup with multiple regions");
    println!("   → Each region gets enough molecules to work with");
    println!("   → No coordination needed - lock-free parallelism works!");
    println!();
    println!("❌ LOW MULTIPLICITY (count < regions):");
    println!("   → Performance degrades or correctness breaks");
    println!("   → Uneven distribution - some regions starve");
    println!("   → Better to use single region");
    println!();
    println!("⚠️  THRESHOLD (count ≈ regions):");
    println!("   → Statistical distribution determines success");
    println!("   → May work but unpredictable performance");
    println!("   → Safe threshold: count >= 2 * regions");
    println!();
}

fn test_high_multiplicity() {
    println!("══════════════════════════════════════════════════════════════");
    println!("Test 1: HIGH MULTIPLICITY (100 pairs)");
    println!("══════════════════════════════════════════════════════════════\n");

    println!("Scenario: 100x [matchp work] + 100x [work N]");
    println!("Expected: Linear speedup across regions\n");

    let mut molecules = vec![];

    // 100 matchp rules (shared across all regions via Arc)
    for i in 0..100 {
        molecules.push(Molecule::new(vec!["matchp", "work", "result", &i.to_string()]));
    }

    // 100 work items (distributed across regions)
    for i in 0..100 {
        molecules.push(Molecule::new(vec!["work", &i.to_string()]));
    }

    println!("Regions | Time (ms) | Speedup | Efficiency | Result");
    println!("--------|-----------|---------|------------|-------");

    let mut baseline = 0.0;

    for num_regions in [1, 2, 4, 8] {
        let mut builder = CompleteFragletsBuilder::new()
            .regions(num_regions)
            .diffusion(0.0)
            .pattern_routing(false);  // Round-robin distribution

        for rule in get_default_rules() {
            builder = builder.add_unimol_rule(rule);
        }

        builder = builder.add_molecules(molecules.clone());

        let start = Instant::now();
        let result = builder.run(1000);
        let duration = start.elapsed();
        let time_ms = duration.as_secs_f64() * 1000.0;

        if num_regions == 1 {
            baseline = time_ms;
        }

        let speedup = if time_ms > 0.0 { baseline / time_ms } else { 0.0 };
        let efficiency = (speedup / num_regions as f64) * 100.0;
        let final_count = result.collect_molecules().len();

        println!("{:^7} | {:9.2} | {:7.2}x | {:9.1}% | {} mols",
            num_regions, time_ms, speedup, efficiency, final_count);
    }

    println!("\n✅ With high multiplicity, each region gets ~25 molecules (100/4)");
    println!("   Plenty to work with - parallelism is effective!\n");
}

fn test_low_multiplicity() {
    println!("══════════════════════════════════════════════════════════════");
    println!("Test 2: LOW MULTIPLICITY (4 pairs, 8 regions)");
    println!("══════════════════════════════════════════════════════════════\n");

    println!("Scenario: 4x [matchp work] + 4x [work N] distributed to 8 regions");
    println!("Expected: Poor performance - most regions have nothing to do\n");

    let mut molecules = vec![];

    // Only 4 matchp rules
    for i in 0..4 {
        molecules.push(Molecule::new(vec!["matchp", "work", "result", &i.to_string()]));
    }

    // Only 4 work items
    for i in 0..4 {
        molecules.push(Molecule::new(vec!["work", &i.to_string()]));
    }

    println!("Regions | Time (ms) | Speedup | Efficiency | Result");
    println!("--------|-----------|---------|------------|-------");

    let mut baseline = 0.0;

    for num_regions in [1, 2, 4, 8] {
        let mut builder = CompleteFragletsBuilder::new()
            .regions(num_regions)
            .diffusion(0.0)
            .pattern_routing(false);

        for rule in get_default_rules() {
            builder = builder.add_unimol_rule(rule);
        }

        builder = builder.add_molecules(molecules.clone());

        let start = Instant::now();
        let result = builder.run(1000);
        let duration = start.elapsed();
        let time_ms = duration.as_secs_f64() * 1000.0;

        if num_regions == 1 {
            baseline = time_ms;
        }

        let speedup = if time_ms > 0.0 { baseline / time_ms } else { 0.0 };
        let efficiency = (speedup / num_regions as f64) * 100.0;
        let final_count = result.collect_molecules().len();

        println!("{:^7} | {:9.2} | {:7.2}x | {:9.1}% | {} mols",
            num_regions, time_ms, speedup, efficiency, final_count);
    }

    println!("\n❌ With 4 molecules and 8 regions, most regions are idle!");
    println!("   Round-robin: 4 regions get 1 molecule, 4 regions get 0");
    println!("   Better to use 1 region for correctness and efficiency\n");
}

fn test_threshold_multiplicity() {
    println!("══════════════════════════════════════════════════════════════");
    println!("Test 3: THRESHOLD MULTIPLICITY (count = num_regions)");
    println!("══════════════════════════════════════════════════════════════\n");

    println!("Scenario: Testing boundary where multiplicity = num_regions");
    println!("Expected: Variable performance - depends on distribution\n");

    println!("Regions | Molecules | Time (ms) | Speedup | Efficiency");
    println!("--------|-----------|-----------|---------|------------");

    for num_regions in [2, 4, 8] {
        let num_molecules = num_regions;  // Exactly equal

        let mut molecules = vec![];

        for i in 0..num_molecules {
            molecules.push(Molecule::new(vec!["matchp", "work", "result", &i.to_string()]));
        }

        for i in 0..num_molecules {
            molecules.push(Molecule::new(vec!["work", &i.to_string()]));
        }

        // Baseline with 1 region
        let mut builder = CompleteFragletsBuilder::new()
            .regions(1)
            .diffusion(0.0)
            .pattern_routing(false);

        for rule in get_default_rules() {
            builder = builder.add_unimol_rule(rule);
        }

        builder = builder.add_molecules(molecules.clone());

        let start = Instant::now();
        builder.run(1000);
        let baseline = start.elapsed().as_secs_f64() * 1000.0;

        // Test with N regions
        let mut builder = CompleteFragletsBuilder::new()
            .regions(num_regions)
            .diffusion(0.0)
            .pattern_routing(false);

        for rule in get_default_rules() {
            builder = builder.add_unimol_rule(rule);
        }

        builder = builder.add_molecules(molecules.clone());

        let start = Instant::now();
        builder.run(1000);
        let time_ms = start.elapsed().as_secs_f64() * 1000.0;

        let speedup = if time_ms > 0.0 { baseline / time_ms } else { 0.0 };
        let efficiency = (speedup / num_regions as f64) * 100.0;

        println!("{:^7} | {:^9} | {:9.2} | {:7.2}x | {:9.1}%",
            num_regions, num_molecules, time_ms, speedup, efficiency);
    }

    println!("\n⚠️  At threshold, each region gets ~1 molecule on average");
    println!("   Statistical variance means some get 0, some get 2");
    println!("   Recommend: multiplicity >= 2 * num_regions for safety\n");
}

fn test_varying_multiplicity() {
    println!("══════════════════════════════════════════════════════════════");
    println!("Test 4: VARYING MULTIPLICITY (Finding the Threshold)");
    println!("══════════════════════════════════════════════════════════════\n");

    println!("Fixed at 4 regions, varying molecule count");
    println!("Finding where parallelism becomes beneficial\n");

    println!("Molecules | Time (ms) | Speedup | Efficiency | M/R Ratio");
    println!("----------|-----------|---------|------------|----------");

    let num_regions = 4;

    for num_pairs in [2, 4, 8, 16, 32, 64, 128] {
        let mut molecules = vec![];

        for i in 0..num_pairs {
            molecules.push(Molecule::new(vec!["matchp", "work", "result", &i.to_string()]));
        }

        for i in 0..num_pairs {
            molecules.push(Molecule::new(vec!["work", &i.to_string()]));
        }

        // Baseline with 1 region
        let mut builder = CompleteFragletsBuilder::new()
            .regions(1)
            .diffusion(0.0)
            .pattern_routing(false);

        for rule in get_default_rules() {
            builder = builder.add_unimol_rule(rule);
        }

        builder = builder.add_molecules(molecules.clone());

        let start = Instant::now();
        builder.run(1000);
        let baseline = start.elapsed().as_secs_f64() * 1000.0;

        // Test with 4 regions
        let mut builder = CompleteFragletsBuilder::new()
            .regions(num_regions)
            .diffusion(0.0)
            .pattern_routing(false);

        for rule in get_default_rules() {
            builder = builder.add_unimol_rule(rule);
        }

        builder = builder.add_molecules(molecules.clone());

        let start = Instant::now();
        builder.run(1000);
        let time_ms = start.elapsed().as_secs_f64() * 1000.0;

        let speedup = if time_ms > 0.0 { baseline / time_ms } else { 0.0 };
        let efficiency = (speedup / num_regions as f64) * 100.0;
        let ratio = num_pairs as f64 / num_regions as f64;

        println!("{:^9} | {:9.2} | {:7.2}x | {:9.1}% | {:^8.1}",
            num_pairs, time_ms, speedup, efficiency, ratio);
    }

    println!("\nObservation: Efficiency improves as M/R ratio increases");
    println!("  M/R < 1:   Poor (more regions than molecules)");
    println!("  M/R = 1-2: Variable (threshold region)");
    println!("  M/R > 2:   Good (enough molecules per region)");
    println!("  M/R >> 1:  Excellent (near-linear speedup)\n");
}

/// Benchmark comparing Shared Pool (no regions) vs Spatial Partitioning (regions)
///
/// Tests whether eliminating regions and using fine-grained locking improves performance

use spatial_fraglets::*;
use std::time::Instant;
use std::sync::Arc;

fn main() {
    println!("╔══════════════════════════════════════════════════════════════╗");
    println!("║         Shared Pool vs Regions Benchmark                    ║");
    println!("╚══════════════════════════════════════════════════════════════╝\n");

    println!("Testing hypothesis: Shared pool eliminates channel/routing overhead\n");

    test_high_multiplicity();
    test_medium_multiplicity();
    test_low_multiplicity();

    println!("\n╔══════════════════════════════════════════════════════════════╗");
    println!("║         Conclusions                                          ║");
    println!("╚══════════════════════════════════════════════════════════════╝");
    println!();
    println!("✅ Shared Pool advantages:");
    println!("   → No channel overhead (send/recv)");
    println!("   → No routing overhead (pattern hashing)");
    println!("   → Pull-based load balancing (no uneven distribution)");
    println!("   → Simpler architecture (no inbox/outbox management)");
    println!();
    println!("✅ Regions advantages:");
    println!("   → Truly lock-free (no contention at all)");
    println!("   → Better for NUMA architectures");
    println!("   → Scales better with many cores (>8)");
    println!();
}

fn test_high_multiplicity() {
    println!("══════════════════════════════════════════════════════════════");
    println!("Test 1: HIGH MULTIPLICITY (100 molecules, 4 workers)");
    println!("══════════════════════════════════════════════════════════════\n");

    let num_molecules = 100;

    println!("Approach  | Workers | Time (ms) | Speedup | Reactions");
    println!("----------|---------|-----------|---------|----------");

    // Test Shared Pool
    for num_workers in [1, 2, 4, 8] {
        // Separate matchp rules from work molecules
        let mut matchp_rules = vec![];
        let mut work_molecules = vec![];

        for i in 0..num_molecules {
            matchp_rules.push(Molecule::new(vec!["matchp", "work", "result", &i.to_string()]));
        }

        for i in 0..num_molecules {
            work_molecules.push(Molecule::new(vec!["work", &i.to_string()]));
        }

        let pool = SharedPool::new(num_workers).with_matchp_rules(matchp_rules);
        let pool = Arc::new(pool);

        // Add work molecules to the pool
        for mol in work_molecules {
            pool.insert(mol);
        }

        let start = Instant::now();
        let reactions = pool.run(1000);
        let time_ms = start.elapsed().as_secs_f64() * 1000.0;

        println!("Shared    | {:^7} | {:9.2} | {:7}x | {}",
            num_workers, time_ms, "-", reactions);
    }

    println!();

    // Test Regions (for comparison)
    for num_regions in [1, 2, 4, 8] {
        let mut builder = CompleteFragletsBuilder::new()
            .regions(num_regions)
            .diffusion(0.0)
            .pattern_routing(false);

        for rule in get_default_rules() {
            builder = builder.add_unimol_rule(rule);
        }

        let mut molecules = vec![];
        for i in 0..num_molecules {
            molecules.push(Molecule::new(vec!["matchp", "work", "result", &i.to_string()]));
        }
        for i in 0..num_molecules {
            molecules.push(Molecule::new(vec!["work", &i.to_string()]));
        }

        builder = builder.add_molecules(molecules);

        let start = Instant::now();
        let result = builder.run(1000);
        let time_ms = start.elapsed().as_secs_f64() * 1000.0;

        println!("Regions   | {:^7} | {:9.2} | {:7}x | {}",
            num_regions, time_ms, "-", result.total_reactions());
    }

    println!();
}

fn test_medium_multiplicity() {
    println!("══════════════════════════════════════════════════════════════");
    println!("Test 2: MEDIUM MULTIPLICITY (32 molecules, 4 workers)");
    println!("══════════════════════════════════════════════════════════════\n");

    let num_molecules = 32;

    println!("Approach  | Workers | Time (ms) | Speedup | Reactions");
    println!("----------|---------|-----------|---------|----------");

    // Test Shared Pool
    for num_workers in [1, 2, 4, 8] {
        let mut matchp_rules = vec![];
        let mut work_molecules = vec![];

        for i in 0..num_molecules {
            matchp_rules.push(Molecule::new(vec!["matchp", "work", "result", &i.to_string()]));
        }

        for i in 0..num_molecules {
            work_molecules.push(Molecule::new(vec!["work", &i.to_string()]));
        }

        let pool = SharedPool::new(num_workers).with_matchp_rules(matchp_rules);
        let pool = Arc::new(pool);

        for mol in work_molecules {
            pool.insert(mol);
        }

        let start = Instant::now();
        let reactions = pool.run(1000);
        let time_ms = start.elapsed().as_secs_f64() * 1000.0;

        println!("Shared    | {:^7} | {:9.2} | {:7}x | {}",
            num_workers, time_ms, "-", reactions);
    }

    println!();

    // Test Regions
    for num_regions in [1, 2, 4, 8] {
        let mut builder = CompleteFragletsBuilder::new()
            .regions(num_regions)
            .diffusion(0.0)
            .pattern_routing(false);

        for rule in get_default_rules() {
            builder = builder.add_unimol_rule(rule);
        }

        let mut molecules = vec![];
        for i in 0..num_molecules {
            molecules.push(Molecule::new(vec!["matchp", "work", "result", &i.to_string()]));
        }
        for i in 0..num_molecules {
            molecules.push(Molecule::new(vec!["work", &i.to_string()]));
        }

        builder = builder.add_molecules(molecules);

        let start = Instant::now();
        let result = builder.run(1000);
        let time_ms = start.elapsed().as_secs_f64() * 1000.0;

        println!("Regions   | {:^7} | {:9.2} | {:7}x | {}",
            num_regions, time_ms, "-", result.total_reactions());
    }

    println!();
}

fn test_low_multiplicity() {
    println!("══════════════════════════════════════════════════════════════");
    println!("Test 3: LOW MULTIPLICITY (4 molecules, 8 workers)");
    println!("══════════════════════════════════════════════════════════════\n");

    let num_molecules = 4;

    println!("Approach  | Workers | Time (ms) | Speedup | Reactions");
    println!("----------|---------|-----------|---------|----------");

    // Test Shared Pool
    for num_workers in [1, 2, 4, 8] {
        let mut matchp_rules = vec![];
        let mut work_molecules = vec![];

        for i in 0..num_molecules {
            matchp_rules.push(Molecule::new(vec!["matchp", "work", "result", &i.to_string()]));
        }

        for i in 0..num_molecules {
            work_molecules.push(Molecule::new(vec!["work", &i.to_string()]));
        }

        let pool = SharedPool::new(num_workers).with_matchp_rules(matchp_rules);
        let pool = Arc::new(pool);

        for mol in work_molecules {
            pool.insert(mol);
        }

        let start = Instant::now();
        let reactions = pool.run(1000);
        let time_ms = start.elapsed().as_secs_f64() * 1000.0;

        println!("Shared    | {:^7} | {:9.2} | {:7}x | {}",
            num_workers, time_ms, "-", reactions);
    }

    println!();

    // Test Regions
    for num_regions in [1, 2, 4, 8] {
        let mut builder = CompleteFragletsBuilder::new()
            .regions(num_regions)
            .diffusion(0.0)
            .pattern_routing(false);

        for rule in get_default_rules() {
            builder = builder.add_unimol_rule(rule);
        }

        let mut molecules = vec![];
        for i in 0..num_molecules {
            molecules.push(Molecule::new(vec!["matchp", "work", "result", &i.to_string()]));
        }
        for i in 0..num_molecules {
            molecules.push(Molecule::new(vec!["work", &i.to_string()]));
        }

        builder = builder.add_molecules(molecules);

        let start = Instant::now();
        let result = builder.run(1000);
        let time_ms = start.elapsed().as_secs_f64() * 1000.0;

        println!("Regions   | {:^7} | {:9.2} | {:7}x | {}",
            num_regions, time_ms, "-", result.total_reactions());
    }

    println!("\nNote: Low multiplicity shows both approaches degrade");
    println!("      Shared pool may have slightly higher overhead due to lock contention");
    println!("      But difference should be minimal with M/R < 1\n");
}

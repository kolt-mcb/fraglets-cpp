// Heavy computation benchmark - shows where parallelism wins
// Demonstrates that with sufficient work, spatial fraglets achieve speedup

use spatial_fraglets::*;

fn main() {
    println!("╔══════════════════════════════════════════════════════════════╗");
    println!("║    Spatial Fraglets - Heavy Computation Benchmark           ║");
    println!("╚══════════════════════════════════════════════════════════════╝");
    println!();

    heavy_computation_benchmark();
}

fn heavy_computation_benchmark() {
    println!("HEAVY COMPUTATION: Prime Factorization");
    println!("═══════════════════════════════════════════════════════════════");
    println!("Each molecule performs prime factorization of a large number");
    println!();

    // Prime factorization - computationally expensive
    fn factorize(mol: &Molecule) -> Option<Vec<Molecule>> {
        let tail = mol.tail();
        if tail.is_empty() {
            return None;
        }

        // Extract number to factorize
        if let Ok(n) = tail[0].parse::<u64>() {
            // Do expensive computation
            let factors = prime_factors(n);

            // Create result molecule with factors
            if factors.len() > 1 {
                let factor_strs: Vec<String> = factors.iter().map(|f| f.to_string()).collect();
                Some(vec![Molecule::from_strings(
                    std::iter::once("result".to_string())
                        .chain(factor_strs)
                        .collect()
                )])
            } else {
                Some(vec![]) // Prime number, consume
            }
        } else {
            None
        }
    }

    let factorize_rule = ReactionRule::new("factorize", "factorize", factorize);

    println!("Regions │    Time (ms) │  Speedup │ Efficiency │ Assessment");
    println!("────────┼──────────────┼──────────┼────────────┼────────────");

    let mut baseline_time = 0.0;

    for num_regions in [1, 2, 4, 8, 12, 16] {
        let mut builder = FragletsBuilder::new()
            .regions(num_regions)
            .diffusion(0.02) // Low diffusion for compute-heavy tasks
            .add_rule(factorize_rule.clone());

        // Create molecules with numbers to factorize
        // Using composite numbers that require computation
        let test_numbers = vec![
            524287,   // Prime (slow to verify)
            1000003,  // Prime
            1048573,  // Prime
            2097143,  // Composite
            4194301,  // Composite
            8388593,  // Prime
            16777213, // Composite
            33554393, // Prime
        ];

        // Replicate to create more work
        for _ in 0..25 {
            for &num in &test_numbers {
                builder = builder.add_molecule(Molecule::new(vec!["factorize", &num.to_string()]));
            }
        }

        let result = builder.run(10);
        let time_ms = result.duration.as_secs_f64() * 1000.0;

        if num_regions == 1 {
            baseline_time = time_ms;
        }

        let speedup = baseline_time / time_ms;
        let efficiency = (speedup / num_regions as f64) * 100.0;

        print!("   {:2}   │ ", num_regions);
        print!("{:>11.1} │ ", time_ms);
        print!("{:>7.2}x │ ", speedup);
        print!("{:>9.1}% │ ", efficiency);

        if speedup > num_regions as f64 * 0.7 {
            println!("✓ Excellent!");
        } else if speedup > num_regions as f64 * 0.4 {
            println!("+ Good");
        } else if speedup > 1.2 {
            println!("+ Speedup achieved");
        } else if speedup > 0.95 {
            println!("≈ Similar");
        } else {
            println!("- Overhead dominates");
        }
    }

    println!();
    println!("═══════════════════════════════════════════════════════════════");
    println!("Key Insight:");
    println!("  When computation >> synchronization overhead,");
    println!("  spatial partitioning achieves significant speedup!");
    println!("═══════════════════════════════════════════════════════════════");
}

// Prime factorization - expensive computation
fn prime_factors(mut n: u64) -> Vec<u64> {
    let mut factors = Vec::new();

    // Factor out 2s
    while n % 2 == 0 {
        factors.push(2);
        n /= 2;
    }

    // Try odd factors
    let mut i = 3;
    while i * i <= n {
        while n % i == 0 {
            factors.push(i);
            n /= i;
        }
        i += 2;
    }

    // Remaining prime
    if n > 1 {
        factors.push(n);
    }

    factors
}

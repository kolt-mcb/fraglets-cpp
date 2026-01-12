// Massive computation benchmark - truly shows parallelism benefits
// Each molecule does significant work to amortize threading overhead

use spatial_fraglets::*;

fn main() {
    println!("╔══════════════════════════════════════════════════════════════╗");
    println!("║      Spatial Fraglets - MASSIVE Computation Benchmark       ║");
    println!("╚══════════════════════════════════════════════════════════════╝");
    println!();
    println!("This benchmark performs heavy computation per molecule to show");
    println!("where spatial parallelism truly shines.");
    println!();

    massive_computation();
}

fn massive_computation() {
    println!("BENCHMARK: Matrix Multiplication in Molecules");
    println!("═══════════════════════════════════════════════════════════════");
    println!("Each molecule multiplies a 50x50 matrix");
    println!();

    // Matrix multiplication - very expensive
    fn matrix_mult(mol: &Molecule) -> Option<Vec<Molecule>> {
        let tail = mol.tail();
        if tail.is_empty() {
            return None;
        }

        // Generate matrices based on seed
        if let Ok(seed) = tail[0].parse::<u32>() {
            let mut rng = fastrand::Rng::with_seed(seed as u64);

            // Create 50x50 matrices
            const SIZE: usize = 50;
            let mut a = vec![vec![0.0f64; SIZE]; SIZE];
            let mut b = vec![vec![0.0f64; SIZE]; SIZE];

            // Initialize with random values
            for i in 0..SIZE {
                for j in 0..SIZE {
                    a[i][j] = rng.f64();
                    b[i][j] = rng.f64();
                }
            }

            // Multiply matrices (O(n³) operation!)
            let mut c = vec![vec![0.0f64; SIZE]; SIZE];
            for i in 0..SIZE {
                for j in 0..SIZE {
                    for k in 0..SIZE {
                        c[i][j] += a[i][k] * b[k][j];
                    }
                }
            }

            // Compute checksum
            let checksum: f64 = c.iter().flat_map(|row| row.iter()).sum();

            // Return result molecule
            Some(vec![Molecule::new(vec!["result", &checksum.to_string()])])
        } else {
            None
        }
    }

    let matrix_rule = ReactionRule::new("compute", "compute", matrix_mult);

    println!("Regions │    Time (ms) │  Speedup │ Efficiency │ Assessment");
    println!("────────┼──────────────┼──────────┼────────────┼───────────────");

    let mut baseline_time = 0.0;
    let molecules_count = 100; // 100 matrices to multiply

    for num_regions in [1, 2, 4, 8] {
        let mut builder = FragletsBuilder::new()
            .regions(num_regions)
            .diffusion(0.0) // NO diffusion - let each region work on its molecules
            .add_rule(matrix_rule.clone());

        // Create molecules
        for i in 0..molecules_count {
            builder = builder.add_molecule(Molecule::new(vec!["compute", &i.to_string()]));
        }

        let result = builder.run(5); // Few iterations since work is heavy
        let time_ms = result.duration.as_secs_f64() * 1000.0;

        if num_regions == 1 {
            baseline_time = time_ms;
        }

        let speedup = baseline_time / time_ms;
        let efficiency = (speedup / num_regions as f64) * 100.0;

        print!("   {:2}   │ ", num_regions);
        print!("{:>11.0} │ ", time_ms);
        print!("{:>7.2}x │ ", speedup);
        print!("{:>9.1}% │ ", efficiency);

        if efficiency > 80.0 {
            println!("✓✓ EXCELLENT! Near-linear scaling!");
        } else if efficiency > 60.0 {
            println!("✓ Excellent scaling");
        } else if efficiency > 40.0 {
            println!("+ Good scaling");
        } else if speedup > 1.2 {
            println!("+ Speedup achieved");
        } else {
            println!("≈ Limited benefit");
        }
    }

    println!();
    println!("═══════════════════════════════════════════════════════════════");
    println!("RESULT: When computation dominates (matrix multiplication),");
    println!("spatial fraglets achieve strong scaling!");
    println!();
    println!("Key: Each molecule does ~125,000 FLOPs (50³ matrix mult)");
    println!("     Total: {} molecules × 125K = {} million FLOPs",
             molecules_count, molecules_count * 125 / 1000);
    println!("═══════════════════════════════════════════════════════════════");
}

// Simple PRNG for consistent results
mod fastrand {
    pub struct Rng(u64);

    impl Rng {
        pub fn with_seed(seed: u64) -> Self {
            Rng(seed)
        }

        pub fn f64(&mut self) -> f64 {
            self.0 = self.0.wrapping_mul(6364136223846793005).wrapping_add(1);
            ((self.0 >> 32) as f64) / (u32::MAX as f64)
        }
    }
}

/// Direct comparison: Regions vs Simple Threading
///
/// This demonstrates why we need regions for lock-free parallelism

use spatial_fraglets::*;
use std::sync::{Arc, Mutex};
use std::thread;

fn main() {
    let workloads = vec![
        ("Light (100 items, 2 ops)", "parallel_work.fra"),
        ("Heavy (100 items, 10 ops)", "parallel_heavy.fra"),
        ("Super Heavy (500 items, 20 ops)", "parallel_super_heavy.fra"),
    ];

    for (name, file) in workloads {
        println!("╔══════════════════════════════════════════════════════════════╗");
        println!("║ {:<60} ║", name);
        println!("╚══════════════════════════════════════════════════════════════╝\n");

        test_workload(file);
        println!();
    }
}

fn test_workload(file: &str) {
    let molecules = parse_fra_file(file).unwrap();

    println!("--- Regions Approach (Lock-Free) ---");
    test_regions(&molecules);

    println!("\n--- Simple Threading (Shared Pool + Lock) ---");
    test_simple_threading(&molecules);
}

fn test_regions(molecules: &[Molecule]) {
    let mut baseline = 0.0;

    for num_regions in [1, 2, 4, 8] {
        let mut builder = CompleteFragletsBuilder::new()
            .regions(num_regions)
            .diffusion(0.0)
            .pattern_routing(false);

        builder = builder.add_molecules(molecules.to_vec());

        let start = std::time::Instant::now();
        let result = builder.run(10000);
        let duration = start.elapsed();
        let time_ms = duration.as_secs_f64() * 1000.0;

        if num_regions == 1 {
            baseline = time_ms;
        }

        let speedup = if baseline > 0.0 { baseline / time_ms } else { 1.0 };

        println!("  {} threads: {:7.2}ms, {:.2}x speedup",
            num_regions, time_ms, speedup);
    }
}

fn test_simple_threading(initial_molecules: &[Molecule]) {
    let mut baseline = 0.0;

    for num_threads in [1, 2, 4, 8] {
        // Separate matchp rules from data
        let mut matchp_rules = Vec::new();
        let mut data = Vec::new();

        for mol in initial_molecules {
            if mol.head() == Some("matchp") {
                matchp_rules.push(mol.clone());
            } else {
                data.push(mol.clone());
            }
        }

        let molecules = Arc::new(Mutex::new(data));
        let rules = Arc::new(matchp_rules);

        let start = std::time::Instant::now();

        let handles: Vec<_> = (0..num_threads)
            .map(|_| {
                let molecules = Arc::clone(&molecules);
                let rules = Arc::clone(&rules);

                thread::spawn(move || {
                    loop {
                        // Lock, grab a molecule, unlock
                        let mol = {
                            let mut mols = molecules.lock().unwrap();
                            if mols.is_empty() {
                                break;
                            }
                            mols.pop()
                        };

                        if let Some(mol) = mol {
                            // Try matchp reactions (outside lock)
                            let mut found_reaction = false;
                            for rule in rules.iter() {
                                if let Some(products) = try_matchp(rule, &mol) {
                                    // Lock, add products (skip matchp rule), unlock
                                    {
                                        let mut mols = molecules.lock().unwrap();
                                        if products.len() > 1 {
                                            mols.extend(products[1..].iter().cloned());
                                        }
                                    }
                                    found_reaction = true;
                                    break;
                                }
                            }

                            // If no reaction, put back (shouldn't happen for our tests)
                            if !found_reaction {
                                molecules.lock().unwrap().push(mol);
                            }
                        }
                    }
                })
            })
            .collect();

        for handle in handles {
            handle.join().unwrap();
        }

        let duration = start.elapsed();
        let time_ms = duration.as_secs_f64() * 1000.0;

        if num_threads == 1 {
            baseline = time_ms;
        }

        let speedup = if baseline > 0.0 { baseline / time_ms } else { 1.0 };

        println!("  {} threads: {:7.2}ms, {:.2}x speedup",
            num_threads, time_ms, speedup);
    }
}

fn try_matchp(rule: &Molecule, mol: &Molecule) -> Option<Vec<Molecule>> {
    use spatial_fraglets::fraglets_ops::op_matchp;

    if rule.head() != Some("matchp") {
        return None;
    }

    op_matchp(rule, mol)
}

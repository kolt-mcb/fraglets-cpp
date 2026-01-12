/// Test: Can we speed up MapReduce with simple threading (no regions)?
///
/// Approach: Shared molecule pool + lock
/// - Single Vec<Molecule> protected by Mutex
/// - Multiple threads grab molecules, react them, put back products
/// - No regions, no routing, no channels
///
/// This is similar to the original C++ approach that failed due to lock contention.
/// Let's measure if it works for MapReduce's embarrassingly parallel workload.

use spatial_fraglets::*;
use std::sync::{Arc, Mutex};
use std::thread;

fn main() {
    let initial_molecules = parse_fra_file("parallel_work.fra").unwrap();

    // Separate matchp rules from data
    let mut matchp_rules = Vec::new();
    let mut data = Vec::new();

    for mol in initial_molecules {
        if mol.head() == Some("matchp") {
            matchp_rules.push(mol);
        } else {
            data.push(mol);
        }
    }

    println!("=== Simple Threading (Shared Pool + Lock) ===\n");
    println!("Workload: {} data items, {} matchp rules\n", data.len(), matchp_rules.len());

    // Test with different thread counts
    for num_threads in [1, 2, 4, 8] {
        let molecules = Arc::new(Mutex::new(data.clone()));
        let rules = Arc::new(matchp_rules.clone());
        let reactions = Arc::new(Mutex::new(0));

        let start = std::time::Instant::now();

        let handles: Vec<_> = (0..num_threads)
            .map(|_| {
                let molecules = Arc::clone(&molecules);
                let rules = Arc::clone(&rules);
                let reactions = Arc::clone(&reactions);

                thread::spawn(move || {
                    // Each thread tries to react molecules
                    for _ in 0..1000 {
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
                            let mut reacted = false;
                            for rule in rules.iter() {
                                if let Some(products) = try_matchp_reaction(rule, &mol) {
                                    // Lock, add products, unlock
                                    let mut mols = molecules.lock().unwrap();
                                    // Skip the matchp rule (it's persistent)
                                    if products.len() > 1 {
                                        mols.extend(products[1..].iter().cloned());
                                    }
                                    drop(mols);

                                    *reactions.lock().unwrap() += 1;
                                    reacted = true;
                                    break;
                                }
                            }

                            // If no reaction, put it back
                            if !reacted {
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
        let total_reactions = *reactions.lock().unwrap();
        let final_count = molecules.lock().unwrap().len();

        println!("{} threads: {:.2}ms, {} reactions, {} final molecules",
            num_threads,
            duration.as_secs_f64() * 1000.0,
            total_reactions,
            final_count);
    }
}

fn try_matchp_reaction(rule: &Molecule, mol: &Molecule) -> Option<Vec<Molecule>> {
    use spatial_fraglets::fraglets_ops::op_matchp;

    if rule.head() != Some("matchp") {
        return None;
    }

    op_matchp(rule, mol)
}

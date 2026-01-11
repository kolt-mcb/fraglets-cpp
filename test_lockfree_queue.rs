/// Alternative Design: Shared Pool with Lock-Free Queue
///
/// Instead of regions (spatial partitioning), use:
/// - Single shared molecule pool (lock-free queue)
/// - Multiple worker threads grab molecules
/// - Process unimol + matchp reactions
/// - Put results back in pool
///
/// No bimolecular "match" reactions between data molecules at all!

use spatial_fraglets::*;
use std::sync::{Arc, Mutex};
use std::thread;
use std::sync::atomic::{AtomicBool, AtomicUsize, Ordering};
use std::collections::VecDeque;

fn main() {
    println!("╔══════════════════════════════════════════════════════════════╗");
    println!("║  Shared Pool (Lock-Free Queue) vs Regions                   ║");
    println!("╚══════════════════════════════════════════════════════════════╝\n");

    test_both_approaches("parallel_work.fra", "Light workload (100 items, 2 ops)");
    test_both_approaches("parallel_heavy.fra", "Heavy workload (100 items, 10 ops)");
    test_both_approaches("sort.fra", "Sequential algorithm (sort)");
}

fn test_both_approaches(file: &str, description: &str) {
    println!("--- {} ---", description);

    let molecules = parse_fra_file(file).unwrap();

    println!("\n1. Shared Lock-Free Queue:");
    test_lockfree_queue(&molecules);

    println!("\n2. Regions Approach:");
    test_regions(&molecules);

    println!();
}

fn test_lockfree_queue(initial_molecules: &[Molecule]) {
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

    for num_threads in [1, 2, 4, 8] {
        let queue = Arc::new(Mutex::new(VecDeque::from(data.clone())));
        let rules = Arc::new(matchp_rules.clone());
        let done = Arc::new(AtomicBool::new(false));
        let reactions = Arc::new(AtomicUsize::new(0));

        let start = std::time::Instant::now();

        let handles: Vec<_> = (0..num_threads)
            .map(|_| {
                let queue = Arc::clone(&queue);
                let rules = Arc::clone(&rules);
                let done = Arc::clone(&done);
                let reactions = Arc::clone(&reactions);

                thread::spawn(move || {
                    let mut idle_count = 0;

                    loop {
                        if done.load(Ordering::Relaxed) {
                            break;
                        }

                        let mol = queue.lock().unwrap().pop_front();

                        if let Some(mol) = mol {
                            idle_count = 0;

                            // Try matchp reactions (outside lock)
                            let mut reacted = false;
                            let mut products_to_add = Vec::new();

                            for rule in rules.iter() {
                                if let Some(products) = try_matchp(rule, &mol) {
                                    // Add products (skip matchp rule)
                                    if products.len() > 1 {
                                        products_to_add.extend(products[1..].iter().cloned());
                                    }
                                    reactions.fetch_add(1, Ordering::Relaxed);
                                    reacted = true;
                                    break;
                                }
                            }

                            // Add results back (with lock)
                            let mut q = queue.lock().unwrap();
                            if !reacted {
                                q.push_back(mol);  // Put back if no reaction
                            } else {
                                for product in products_to_add {
                                    q.push_back(product);
                                }
                            }
                        } else {
                            idle_count += 1;
                            if idle_count > 1000 {
                                // Queue empty for a while, probably done
                                done.store(true, Ordering::Relaxed);
                                break;
                            }
                            thread::yield_now();
                        }
                    }
                })
            })
            .collect();

        for handle in handles {
            handle.join().unwrap();
        }

        let duration = start.elapsed();
        let total_reactions = reactions.load(Ordering::Relaxed);

        println!("  {} threads: {:7.2}ms, {} reactions",
            num_threads,
            duration.as_secs_f64() * 1000.0,
            total_reactions);
    }
}

fn test_regions(molecules: &[Molecule]) {
    for num_regions in [1, 2, 4, 8] {
        let mut builder = CompleteFragletsBuilder::new()
            .regions(num_regions)
            .diffusion(0.0)
            .pattern_routing(false);

        // Add unimol rules if needed
        if molecules.iter().any(|m| m.head() == Some("sort")) {
            for rule in get_default_rules() {
                builder = builder.add_unimol_rule(rule);
            }
        }

        builder = builder.add_molecules(molecules.to_vec());

        let start = std::time::Instant::now();
        let result = builder.run(10000);
        let duration = start.elapsed();

        println!("  {} regions: {:7.2}ms, {} reactions",
            num_regions,
            duration.as_secs_f64() * 1000.0,
            result.total_reactions());
    }
}

fn try_matchp(rule: &Molecule, mol: &Molecule) -> Option<Vec<Molecule>> {
    use spatial_fraglets::fraglets_ops::op_matchp;

    if rule.head() != Some("matchp") {
        return None;
    }

    op_matchp(rule, mol)
}

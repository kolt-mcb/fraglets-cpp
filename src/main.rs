// Spatial Fraglets - Demonstration Program
// Shows lock-free parallel execution with MapReduce example

use spatial_fraglets::*;
use std::collections::HashMap;
use std::thread;
use std::time::Instant;

fn main() {
    println!("╔══════════════════════════════════════════════════════════════╗");
    println!("║    Spatial Fraglets - Lock-Free Parallel Architecture       ║");
    println!("╚══════════════════════════════════════════════════════════════╝");
    println!();

    demo_basic();
    println!();
    demo_mapreduce();
}

/// Demo 1: Basic fraglets with reactions
fn demo_basic() {
    println!("DEMO 1: Basic Spatial Fraglets");
    println!("═══════════════════════════════════════════════════════════════");
    println!();

    // Create reactions
    let nul_rule = ReactionRule::new("nul", "nul", nul);
    let dup_rule = ReactionRule::new("dup", "dup", dup);
    let split_rule = ReactionRule::new("split", "split", split);

    // Test with 1, 2, 4, 8 regions
    for num_regions in [1, 2, 4, 8] {
        print!("  {} region{}: ", num_regions, if num_regions == 1 { " " } else { "s" });

        // Create initial molecules
        let molecules = vec![
            Molecule::new(vec!["dup", "a", "b", "c"]),
            Molecule::new(vec!["split", "x", "y", "z"]),
            Molecule::new(vec!["nul"]),
        ];

        let result = FragletsBuilder::new()
            .regions(num_regions)
            .diffusion(0.1)
            .add_rule(nul_rule.clone())
            .add_rule(dup_rule.clone())
            .add_rule(split_rule.clone())
            .add_molecule(molecules[0].clone())
            .add_molecule(molecules[1].clone())
            .add_molecule(molecules[2].clone())
            .run(100);

        println!(
            "{:>4} reactions, {:>2} molecules remaining, {:>4.1}ms",
            result.total_reactions(),
            result.total_molecules(),
            result.duration.as_secs_f64() * 1000.0
        );
    }

    println!();
    println!("✓ All configurations completed successfully");
}

/// Demo 2: MapReduce word counting
fn demo_mapreduce() {
    println!("DEMO 2: MapReduce Word Count (Native Rust)");
    println!("═══════════════════════════════════════════════════════════════");
    println!();

    // Generate test text
    let text = "the quick brown fox jumps over the lazy dog ".repeat(10000);
    println!("Text length: {} characters", text.len());
    println!("Word count: ~{} words", text.split_whitespace().count());
    println!();

    println!("Sequential vs Parallel Comparison:");
    println!("───────────────────────────────────────────────────────────────");

    // Sequential (1 worker)
    print!("  1 worker (sequential): ");
    let (seq_result, seq_time) = mapreduce_wordcount(&text, 1);
    println!("{:>6.2}ms - {} unique words", seq_time * 1000.0, seq_result.len());

    // Parallel configurations
    for workers in [2, 4, 8] {
        print!("  {} workers (parallel):  ", workers);
        let (par_result, par_time) = mapreduce_wordcount(&text, workers);
        let speedup = seq_time / par_time;
        let efficiency = (speedup / workers as f64) * 100.0;

        println!(
            "{:>6.2}ms - speedup: {:.2}x, efficiency: {:.1}%",
            par_time * 1000.0,
            speedup,
            efficiency
        );

        // Verify correctness
        assert_eq!(seq_result, par_result, "Results must match!");
    }

    println!();
    println!("✓ MapReduce completed - results verified correct");
}

/// MapReduce word count implementation
fn mapreduce_wordcount(text: &str, num_workers: usize) -> (HashMap<String, usize>, f64) {
    let start = Instant::now();

    // PARTITION: Split text into chunks
    let words: Vec<&str> = text.split_whitespace().collect();
    let chunk_size = (words.len() + num_workers - 1) / num_workers;

    // MAP: Each worker counts its chunk (PARALLEL!)
    let handles: Vec<_> = (0..num_workers)
        .map(|worker_id| {
            let start_idx = worker_id * chunk_size;
            let end_idx = (start_idx + chunk_size).min(words.len());
            let chunk: Vec<String> = words[start_idx..end_idx]
                .iter()
                .map(|s| s.to_string())
                .collect();

            thread::spawn(move || {
                let mut counts = HashMap::new();
                for word in chunk {
                    *counts.entry(word.to_lowercase()).or_insert(0) += 1;
                }
                counts
            })
        })
        .collect();

    // REDUCE: Merge all counts
    let mut global_counts = HashMap::new();
    for handle in handles {
        let local_counts = handle.join().unwrap();
        for (word, count) in local_counts {
            *global_counts.entry(word).or_insert(0) += count;
        }
    }

    let duration = start.elapsed().as_secs_f64();
    (global_counts, duration)
}

use std::collections::hash_map::DefaultHasher;
use std::hash::{Hash, Hasher};

fn hash_pattern(pattern: &str) -> usize {
    let mut hasher = DefaultHasher::new();
    pattern.hash(&mut hasher);
    hasher.finish() as usize
}

fn main() {
    let num_regions = 4;

    println!("Pattern distribution for {} regions:", num_regions);
    println!();

    // Check distribution of patterns
    let patterns = vec!["work", "result", "done", "matchp"];

    for pattern in &patterns {
        let target = hash_pattern(pattern) % num_regions;
        println!("  {} -> region {}", pattern, target);
    }

    println!();
    println!("Work item distribution:");

    // Count how many work items go to each region
    let mut counts = vec![0; num_regions];
    for i in 1..=100 {
        counts[hash_pattern("work") % num_regions] += 1;
    }

    for (region, count) in counts.iter().enumerate() {
        println!("  Region {}: {} items", region, count);
    }
}

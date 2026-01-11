use std::collections::hash_map::DefaultHasher;
use std::hash::{Hash, Hasher};

fn hash_pattern(pattern: &str) -> usize {
    let mut hasher = DefaultHasher::new();
    pattern.hash(&mut hasher);
    hasher.finish() as usize
}

fn main() {
    let num_regions = 4;

    println!("Sort.fra pattern distribution for {} regions:\n", num_regions);

    let patterns = vec![
        "sorted", "sort", "continue", "remain", "getmin",
        "min", "split", "match", "tosorted",
        "len1", "getmin2", "min2", "d1", "d11", "getmin3",
        "islt", "nlt", "r1", "matchp"
    ];

    for pattern in &patterns {
        let target = hash_pattern(pattern) % num_regions;
        println!("  {:12} -> region {}", pattern, target);
    }

    println!("\nKey insight:");
    println!("  - 'remain' molecules go to region {}", hash_pattern("remain") % num_regions);
    println!("  - 'match' molecules go to region {}", hash_pattern("match") % num_regions);
    println!("  - They need to react together!");
}

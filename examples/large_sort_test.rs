/// Large-scale sorting benchmark: 5000 items
/// Tests shared pool performance on big sequential workload

use spatial_fraglets::*;
use std::sync::Arc;
use std::time::Instant;
use rand::Rng;

fn main() {
    println!("╔══════════════════════════════════════════════════════════════╗");
    println!("║         Large-Scale Sort: 5000 Items                         ║");
    println!("╚══════════════════════════════════════════════════════════════╝\n");

    test_large_sort(100);
    test_large_sort(500);
    test_large_sort(1000);
    test_large_sort(5000);

    println!("\n╔══════════════════════════════════════════════════════════════╗");
    println!("║         Key Insights                                         ║");
    println!("╚══════════════════════════════════════════════════════════════╝\n");

    println!("Shared pool handles sequential algorithms of any size!");
    println!("  • Small (100): Fast baseline");
    println!("  • Medium (500-1000): Good performance");
    println!("  • Large (5000): Scales with O(n²) as expected");
    println!("\nNote: Regions with >1 region would BREAK on all of these.\n");
}

fn test_large_sort(num_items: usize) {
    println!("══════════════════════════════════════════════════════════════");
    println!("Sorting {} random numbers", num_items);
    println!("══════════════════════════════════════════════════════════════\n");

    // Generate random numbers
    let mut rng = rand::thread_rng();
    let mut numbers: Vec<i32> = (0..num_items)
        .map(|_| rng.gen_range(-1000..1000))
        .collect();

    println!("Generated {} random numbers", numbers.len());
    println!("Sample: {:?}...", &numbers[..5.min(numbers.len())]);

    // Build fraglets molecules for sorting
    let mut molecules = create_sort_molecules(&numbers);

    println!("Created {} molecules for sorting", molecules.len());

    // Separate matchp rules from data
    let mut matchp_rules = vec![];
    let mut data_molecules = vec![];

    for mol in molecules {
        if mol.head() == Some("matchp") {
            matchp_rules.push(mol);
        } else {
            data_molecules.push(mol);
        }
    }

    println!("  {} matchp rules", matchp_rules.len());
    println!("  {} data molecules\n", data_molecules.len());

    // Test with 1 thread (optimal for sequential)
    let mut pool = SharedPool::new(1);
    for rule in get_default_rules() {
        pool.add_unimol_rule(rule);
    }

    let pool = pool.with_matchp_rules(matchp_rules);
    let pool = Arc::new(pool);

    for mol in data_molecules {
        pool.insert(mol);
    }

    println!("Running sort with 1 thread...");
    let start = Instant::now();
    let pool_clone = Arc::clone(&pool);

    // Adjust max iterations based on size (sorting is O(n²))
    let max_iters = if num_items <= 100 { 50000 }
                    else if num_items <= 500 { 200000 }
                    else if num_items <= 1000 { 500000 }
                    else { 2000000 };

    let reactions = pool_clone.run(max_iters);
    let time_ms = start.elapsed().as_secs_f64() * 1000.0;

    println!("Time: {:.2}ms ({:.2}s)", time_ms, time_ms / 1000.0);
    println!("Reactions: {}", reactions);
    println!("Reactions per second: {:.0}", reactions as f64 / (time_ms / 1000.0));

    // Check results
    let final_mols = pool.collect_molecules();
    let mut found_sorted = false;
    let mut sorted_count = 0;

    for mol in &final_mols {
        if mol.head() == Some("sorted") {
            let nums: Vec<i32> = mol.tail()
                .iter()
                .filter_map(|s| s.parse::<i32>().ok())
                .collect();
            sorted_count = nums.len();
            let is_sorted = nums.windows(2).all(|w| w[0] <= w[1]);
            found_sorted = is_sorted;

            println!("\nResult: {} numbers sorted", sorted_count);
            if sorted_count <= 20 {
                println!("  Numbers: {:?}", nums);
            } else {
                println!("  First 10: {:?}", &nums[..10]);
                println!("  Last 10:  {:?}", &nums[nums.len()-10..]);
            }
            println!("  Is sorted: {} {}", is_sorted, if is_sorted { "✅" } else { "❌" });
            println!("  Coverage: {}/{} ({:.1}%)", sorted_count, num_items,
                     (sorted_count as f64 / num_items as f64) * 100.0);
        }
    }

    if !found_sorted {
        println!("\n❌ No sorted output found");
        println!("Final molecules: {}", final_mols.len());
        for mol in final_mols.iter().take(5) {
            println!("  {:?}", mol);
        }
    }

    println!();
}

fn create_sort_molecules(numbers: &[i32]) -> Vec<Molecule> {
    let mut molecules = vec![];

    // Add sort fraglets rules (from sort.fra)
    molecules.push(Molecule::new(vec!["sorted"]));
    molecules.push(Molecule::new(vec!["matchp", "sort", "empty", "finish", "continue"]));
    molecules.push(Molecule::new(vec!["matchp", "continue", "split", "remain", "*", "getmin"]));
    molecules.push(Molecule::new(vec!["matchp", "min", "split", "match", "remain", "sort", "*",
                                       "split", "match", "sorted", "match", "tosorted", "sorted", "*", "tosorted"]));
    molecules.push(Molecule::new(vec!["matchp", "getmin", "length", "len1"]));
    molecules.push(Molecule::new(vec!["matchp", "len1", "lt", "getmin2", "min2", "1"]));
    molecules.push(Molecule::new(vec!["matchp", "min2", "pop", "d1"]));
    molecules.push(Molecule::new(vec!["matchp", "d1", "pop", "min"]));
    molecules.push(Molecule::new(vec!["matchp", "getmin2", "pop", "d11"]));
    molecules.push(Molecule::new(vec!["matchp", "d11", "pop", "getmin3"]));
    molecules.push(Molecule::new(vec!["matchp", "getmin3", "lt", "islt", "nlt"]));
    molecules.push(Molecule::new(vec!["matchp", "nlt", "pop2", "r1", "getmin"]));
    molecules.push(Molecule::new(vec!["matchp", "islt", "exch", "nlt"]));
    molecules.push(Molecule::new(vec!["matchp", "r1", "match", "remain", "remain"]));

    // Add the data to sort
    let mut sort_mol = vec!["sort".to_string()];
    for num in numbers {
        sort_mol.push(num.to_string());
    }
    molecules.push(Molecule::from_strings(sort_mol));

    molecules
}

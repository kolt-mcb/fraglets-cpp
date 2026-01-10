// Fraglets - Main CLI compatible with C++ version
// Usage: fraglets <file.fra> [options]

use spatial_fraglets::*;
use std::env;

fn main() {
    let args: Vec<String> = env::args().collect();

    if args.len() < 2 {
        println!("Usage: {} <file.fra> [options]", args[0]);
        println!();
        println!("Options:");
        println!("  --iterations <n>    Maximum iterations (default: 1000)");
        println!("  --regions <n>       Number of parallel regions (default: 4)");
        println!("  --diffusion <rate>  Molecule migration rate 0.0-1.0 (default: 0.05)");
        println!("  --quiet             Suppress output");
        println!("  --trace             Show final molecule state");
        std::process::exit(1);
    }

    let filename = &args[1];
    let mut iterations = 1000;
    let mut regions = 4;
    let mut diffusion = 0.05;
    let mut quiet = false;
    let mut trace = false;

    // Parse arguments
    let mut i = 2;
    while i < args.len() {
        match args[i].as_str() {
            "--iterations" => {
                i += 1;
                if i < args.len() {
                    iterations = args[i].parse().unwrap_or(1000);
                }
            }
            "--regions" => {
                i += 1;
                if i < args.len() {
                    regions = args[i].parse().unwrap_or(4);
                }
            }
            "--diffusion" => {
                i += 1;
                if i < args.len() {
                    diffusion = args[i].parse().unwrap_or(0.05);
                }
            }
            "--quiet" => quiet = true,
            "--trace" => trace = true,
            _ => {
                eprintln!("Unknown option: {}", args[i]);
            }
        }
        i += 1;
    }

    // Parse .fra file
    if !quiet {
        println!("Loading fraglets from: {}", filename);
    }

    let molecules = match parse_fra_file(filename) {
        Ok(mols) => mols,
        Err(e) => {
            eprintln!("Error parsing file: {}", e);
            std::process::exit(1);
        }
    };

    if !quiet {
        println!("Loaded {} molecules", molecules.len());
        println!("Regions: {}, Iterations: {}, Diffusion: {:.3}", regions, iterations, diffusion);
        println!();
    }

    // Build fraglets system with default rules
    let mut builder = CompleteFragletsBuilder::new()
        .regions(regions)
        .diffusion(diffusion);

    // Add all default unimol rules
    for rule in get_default_rules() {
        builder = builder.add_unimol_rule(rule);
    }

    // Add molecules
    builder = builder.add_molecules(molecules);

    // Run
    if !quiet {
        println!("Running...");
    }

    let result = builder.run(iterations);

    // Output results
    if !quiet {
        println!();
        println!("Completed in {:.2}ms", result.duration.as_secs_f64() * 1000.0);
        println!("Total reactions: {}", result.total_reactions());
        println!("Remaining molecules: {}", result.total_molecules());
    }

    if trace {
        println!();
        println!("=== Final State ===");
        for (region_id, region_result) in result.regions.iter().enumerate() {
            if !region_result.remaining_molecules.is_empty() {
                println!("Region {}:", region_id);
                for mol in &region_result.remaining_molecules {
                    println!("  {:?}", mol.symbols);
                }
            }
        }
    }

    // Print just the final molecules for compatibility
    if !quiet && !trace {
        let final_mols = result.collect_molecules();
        if !final_mols.is_empty() {
            println!();
            println!("Final molecules:");
            for mol in final_mols {
                println!("  {:?}", mol.symbols);
            }
        }
    }
}

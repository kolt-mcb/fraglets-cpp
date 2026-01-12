use spatial_fraglets::*;
use std::env;

fn main() {
    let args: Vec<String> = env::args().collect();
    if args.len() < 2 {
        eprintln!("Usage: {} <file.fra>", args[0]);
        std::process::exit(1);
    }

    let molecules = parse_fra_file(&args[1]).unwrap();

    println!("Initial: {} molecules", molecules.len());
    for mol in &molecules {
        println!("  {:?}", mol.symbols);
    }

    let mut builder = CompleteFragletsBuilder::new()
        .regions(1)
        .diffusion(0.0);

    for rule in get_default_rules() {
        builder = builder.add_unimol_rule(rule);
    }

    builder = builder.add_molecules(molecules);
    let result = builder.run(100);

    println!("\n=== Reactions ===");
    for region in &result.regions {
        for (i, event) in region.reaction_history.iter().enumerate() {
            println!("\nReaction {}:", i+1);
            println!("  Type: {:?}", event.reaction_type);
            println!("  Reactants:");
            for r in &event.reactants {
                println!("    {:?}", r.symbols);
            }
            println!("  Products:");
            for p in &event.products {
                println!("    {:?}", p.symbols);
            }
        }
    }

    println!("\n=== Final: {} molecules ===", result.total_molecules());
    for mol in result.collect_molecules() {
        println!("  {:?}", mol.symbols);
    }
}

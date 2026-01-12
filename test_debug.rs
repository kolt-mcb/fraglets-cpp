// Quick debug test
use spatial_fraglets::*;

fn main() {
    let molecules = vec![
        Molecule::from_strings(vec!["matchp".to_string(), "sort".to_string(), "empty".to_string(), "finish".to_string(), "continue".to_string()]),
        Molecule::from_strings(vec!["sort".to_string(), "5".to_string(), "3".to_string()]),
    ];

    let mut builder = CompleteFragletsBuilder::new()
        .regions(1)
        .diffusion(0.0);

    for rule in get_default_rules() {
        builder = builder.add_unimol_rule(rule);
    }

    builder = builder.add_molecules(molecules.clone());

    println!("Initial molecules:");
    for mol in &molecules {
        println!("  {:?}", mol.symbols);
    }

    let result = builder.run(100);

    println!("\nFinal molecules:");
    for mol in result.collect_molecules() {
        println!("  {:?}", mol.symbols);
    }

    println!("\nReaction history:");
    for (i, region) in result.regions.iter().enumerate() {
        println!("Region {}:", i);
        for (j, event) in region.reaction_history.iter().enumerate() {
            println!("  Reaction {}:", j);
            println!("    Reactants:");
            for r in &event.reactants {
                println!("      {:?}", r.symbols);
            }
            println!("    Products:");
            for p in &event.products {
                println!("      {:?}", p.symbols);
            }
            println!("    Type: {:?}", event.reaction_type);
        }
    }
}

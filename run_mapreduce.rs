use spatial_fraglets::*;
use std::fs::File;
use std::io::Write;

fn main() {
    let molecules = parse_fra_file("mapreduce.fra").unwrap();

    println!("Initial molecules: {}", molecules.len());
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
    let result = builder.run(1000);

    println!("\n=== Execution Results ===");
    println!("Total reactions: {}", result.total_reactions());
    println!("Final molecules: {}", result.total_molecules());

    println!("\n=== Final State ===");
    for mol in result.collect_molecules() {
        println!("  {:?}", mol.symbols);
    }

    // Generate graphviz
    println!("\n=== Generating Graphviz ===");
    let mut dot = String::new();
    dot.push_str("digraph mapreduce {\n");
    dot.push_str("  rankdir=LR;\n");
    dot.push_str("  node [shape=box, style=rounded];\n\n");

    let mut node_id = 0;
    let mut get_node_id = || {
        let id = node_id;
        node_id += 1;
        id
    };

    for region in &result.regions {
        for event in &region.reaction_history {
            let reactant_ids: Vec<_> = event.reactants.iter()
                .map(|_| get_node_id())
                .collect();

            let product_ids: Vec<_> = event.products.iter()
                .map(|_| get_node_id())
                .collect();

            // Create reactant nodes
            for (id, mol) in reactant_ids.iter().zip(&event.reactants) {
                let label = format!("{:?}", mol.symbols)
                    .replace("\"", "\\\"")
                    .replace("[", "")
                    .replace("]", "");
                dot.push_str(&format!("  n{} [label=\"{}\"];\n", id, label));
            }

            // Create product nodes
            for (id, mol) in product_ids.iter().zip(&event.products) {
                let label = format!("{:?}", mol.symbols)
                    .replace("\"", "\\\"")
                    .replace("[", "")
                    .replace("]", "");
                dot.push_str(&format!("  n{} [label=\"{}\", style=\"rounded,filled\", fillcolor=lightblue];\n", id, label));
            }

            // Create reaction node
            let reaction_id = get_node_id();
            let reaction_label = match event.reaction_type {
                ReactionType::Unimol => "unimol",
                ReactionType::Matchp => "matchp",
                ReactionType::Bimol => "bimol",
            };
            dot.push_str(&format!("  n{} [label=\"{}\", shape=circle, fillcolor=yellow, style=filled];\n",
                reaction_id, reaction_label));

            // Connect reactants to reaction
            for id in &reactant_ids {
                dot.push_str(&format!("  n{} -> n{};\n", id, reaction_id));
            }

            // Connect reaction to products
            for id in &product_ids {
                dot.push_str(&format!("  n{} -> n{};\n", reaction_id, id));
            }

            dot.push_str("\n");
        }
    }

    dot.push_str("}\n");

    let mut file = File::create("mapreduce.dot").unwrap();
    file.write_all(dot.as_bytes()).unwrap();
    println!("Wrote mapreduce.dot");
}

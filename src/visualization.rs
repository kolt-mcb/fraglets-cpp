// Graphviz visualization for molecule reaction networks

use crate::{Molecule, RunResult};
use std::collections::{HashMap, HashSet};
use std::fs::File;
use std::io::Write;

/// Generate a DOT file showing molecule reaction network
pub fn generate_reaction_network(
    initial_molecules: &[Molecule],
    result: &RunResult,
    output_path: &str,
) -> Result<(), std::io::Error> {
    let mut dot = String::new();

    // Header
    dot.push_str("digraph ReactionNetwork {\n");
    dot.push_str("  rankdir=LR;\n");
    dot.push_str("  node [shape=box, style=rounded];\n\n");

    // Collect all unique molecules
    let mut all_molecules = Vec::new();
    all_molecules.extend(initial_molecules.iter().cloned());
    all_molecules.extend(result.collect_molecules());

    let mut molecule_ids: HashMap<String, usize> = HashMap::new();
    let mut id_counter = 0;

    // Add initial molecules
    dot.push_str("  // Initial molecules\n");
    for mol in initial_molecules {
        let mol_str = format_molecule(mol);
        if !molecule_ids.contains_key(&mol_str) {
            molecule_ids.insert(mol_str.clone(), id_counter);
            dot.push_str(&format!(
                "  n{} [label=\"{}\", fillcolor=lightblue, style=\"rounded,filled\"];\n",
                id_counter,
                escape_label(&mol_str)
            ));
            id_counter += 1;
        }
    }

    // Add final molecules
    dot.push_str("\n  // Final molecules\n");
    for mol in result.collect_molecules() {
        let mol_str = format_molecule(&mol);
        if !molecule_ids.contains_key(&mol_str) {
            molecule_ids.insert(mol_str.clone(), id_counter);
            dot.push_str(&format!(
                "  n{} [label=\"{}\", fillcolor=lightgreen, style=\"rounded,filled\"];\n",
                id_counter,
                escape_label(&mol_str)
            ));
            id_counter += 1;
        }
    }

    // Add region info
    dot.push_str("\n  // Regions\n");
    for region in &result.regions {
        if !region.remaining_molecules.is_empty() {
            dot.push_str(&format!(
                "  subgraph cluster_region_{} {{\n",
                region.id
            ));
            dot.push_str(&format!("    label=\"Region {} ({} reactions)\";\n", region.id, region.reactions));
            dot.push_str("    style=dashed;\n");

            for mol in &region.remaining_molecules {
                let mol_str = format_molecule(mol);
                if let Some(&id) = molecule_ids.get(&mol_str) {
                    dot.push_str(&format!("    n{};\n", id));
                }
            }

            dot.push_str("  }\n");
        }
    }

    dot.push_str("\n  // Statistics\n");
    dot.push_str(&format!(
        "  stats [shape=note, label=\"Total Reactions: {}\\nTime: {:.2}ms\\nRegions: {}\"];\n",
        result.total_reactions(),
        result.duration.as_secs_f64() * 1000.0,
        result.regions.len()
    ));

    dot.push_str("}\n");

    // Write to file
    let mut file = File::create(output_path)?;
    file.write_all(dot.as_bytes())?;

    Ok(())
}

/// Generate DOT file showing molecule flow between regions
pub fn generate_region_flow(
    result: &RunResult,
    output_path: &str,
) -> Result<(), std::io::Error> {
    let mut dot = String::new();

    dot.push_str("digraph RegionFlow {\n");
    dot.push_str("  rankdir=TB;\n");
    dot.push_str("  node [shape=circle, style=filled];\n\n");

    // Create region nodes
    for region in &result.regions {
        let color = if region.reactions > 0 {
            "lightgreen"
        } else {
            "lightgray"
        };

        dot.push_str(&format!(
            "  r{} [label=\"Region {}\\n{} reactions\\n{} molecules\", fillcolor={}];\n",
            region.id,
            region.id,
            region.reactions,
            region.remaining_molecules.len(),
            color
        ));
    }

    // Add edges showing potential message passing
    dot.push_str("\n  // Potential molecule migration paths\n");
    for i in 0..result.regions.len() {
        for j in 0..result.regions.len() {
            if i != j {
                dot.push_str(&format!("  r{} -> r{} [style=dashed, color=gray];\n", i, j));
            }
        }
    }

    dot.push_str("}\n");

    let mut file = File::create(output_path)?;
    file.write_all(dot.as_bytes())?;

    Ok(())
}

/// Generate DOT file showing operation types
pub fn generate_operation_graph(
    molecules: &[Molecule],
    output_path: &str,
) -> Result<(), std::io::Error> {
    let mut dot = String::new();

    dot.push_str("digraph Operations {\n");
    dot.push_str("  rankdir=LR;\n");
    dot.push_str("  node [shape=box];\n\n");

    // Group by operation type
    let mut operations: HashMap<String, Vec<String>> = HashMap::new();

    for mol in molecules {
        if let Some(op) = mol.head() {
            let tail = mol.tail().join(" ");
            operations
                .entry(op.to_string())
                .or_insert_with(Vec::new)
                .push(tail);
        }
    }

    // Create operation nodes
    for (op, instances) in &operations {
        let color = match op.as_str() {
            "matchp" | "match" => "lightblue",
            "nul" => "lightcoral",
            "pop" | "pop2" => "lightyellow",
            "dup" | "fork" => "lightgreen",
            "partition" | "merge" => "plum",
            _ => "white",
        };

        dot.push_str(&format!(
            "  \"{}\" [label=\"{}\\n({} instances)\", fillcolor={}, style=filled];\n",
            op,
            op,
            instances.len(),
            color
        ));
    }

    // Connect bimol operations
    if operations.contains_key("matchp") || operations.contains_key("match") {
        dot.push_str("\n  // Bimolecular reactions\n");
        for op in ["matchp", "match"] {
            if operations.contains_key(op) {
                dot.push_str(&format!("  \"{}\" -> \"reaction\" [label=\"reacts with\"];\n", op));
            }
        }
        dot.push_str("  \"reaction\" [shape=ellipse, fillcolor=yellow, style=filled];\n");
    }

    dot.push_str("}\n");

    let mut file = File::create(output_path)?;
    file.write_all(dot.as_bytes())?;

    Ok(())
}

/// Format molecule for display
fn format_molecule(mol: &Molecule) -> String {
    mol.symbols.join(" ")
}

/// Escape special characters for DOT labels
fn escape_label(s: &str) -> String {
    s.replace('\\', "\\\\")
        .replace('"', "\\\"")
        .replace('\n', "\\n")
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_escape_label() {
        assert_eq!(escape_label("hello"), "hello");
        assert_eq!(escape_label("hello\"world"), "hello\\\"world");
    }

    #[test]
    fn test_format_molecule() {
        let mol = Molecule::new(vec!["matchp", "sort", "empty"]);
        assert_eq!(format_molecule(&mol), "matchp sort empty");
    }
}

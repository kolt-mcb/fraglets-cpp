// Complete fraglets operations implementation
// Compatible with original C++ fraglets

use crate::{Molecule, ReactionRule};

// ============================================================================
// UNIMOLECULAR OPERATIONS
// ============================================================================

/// nul - molecule disappears
pub fn op_nul(_mol: &Molecule) -> Option<Vec<Molecule>> {
    Some(vec![]) // Molecule disappears
}

/// pop - removes first symbol
pub fn op_pop(mol: &Molecule) -> Option<Vec<Molecule>> {
    if mol.symbols.len() > 1 {
        Some(vec![Molecule::from_strings(mol.tail())])
    } else {
        Some(vec![]) // Disappears if only head
    }
}

/// pop2 - removes first two symbols
pub fn op_pop2(mol: &Molecule) -> Option<Vec<Molecule>> {
    if mol.symbols.len() > 2 {
        let tail: Vec<String> = mol.symbols[2..].to_vec();
        Some(vec![Molecule::from_strings(tail)])
    } else {
        Some(vec![]) // Disappears if too short
    }
}

/// dup - duplicates the tail
pub fn op_dup(mol: &Molecule) -> Option<Vec<Molecule>> {
    let tail = mol.tail();
    if !tail.is_empty() {
        Some(vec![
            Molecule::from_strings(tail.clone()),
            Molecule::from_strings(tail),
        ])
    } else {
        Some(vec![])
    }
}

/// exch - exchanges first two symbols after head
pub fn op_exch(mol: &Molecule) -> Option<Vec<Molecule>> {
    if mol.symbols.len() >= 3 {
        let mut new_symbols = vec![mol.symbols[0].clone(), mol.symbols[2].clone(), mol.symbols[1].clone()];
        new_symbols.extend_from_slice(&mol.symbols[3..]);
        Some(vec![Molecule::from_strings(new_symbols)])
    } else {
        None
    }
}

/// split - breaks into individual symbol molecules
pub fn op_split(mol: &Molecule) -> Option<Vec<Molecule>> {
    let tail = mol.tail();
    if !tail.is_empty() {
        Some(tail.into_iter().map(|s| Molecule::new(vec![&s])).collect())
    } else {
        Some(vec![])
    }
}

/// fork - duplicates entire molecule
pub fn op_fork(mol: &Molecule) -> Option<Vec<Molecule>> {
    Some(vec![
        Molecule::from_strings(mol.symbols.clone()),
        Molecule::from_strings(mol.symbols.clone()),
    ])
}

/// nop - removes itself and returns the tail
pub fn op_nop(mol: &Molecule) -> Option<Vec<Molecule>> {
    let tail = mol.tail();
    if !tail.is_empty() {
        Some(vec![Molecule::from_strings(tail)])
    } else {
        None // Molecule disappears if only [nop]
    }
}

/// empty - creates empty marker molecule
pub fn op_empty(mol: &Molecule) -> Option<Vec<Molecule>> {
    // [empty tag ...] where if size > 3, removes "empty" and "tag"
    if mol.symbols.len() > 3 {
        // Return everything from position 2 onwards
        let result: Vec<String> = mol.symbols[2..].to_vec();
        Some(vec![Molecule::from_strings(result)])
    } else {
        // Size <= 3: no reaction
        None
    }
}

/// length - returns length of tail
pub fn op_length(mol: &Molecule) -> Option<Vec<Molecule>> {
    if mol.symbols.len() >= 2 {
        let tail_len = mol.symbols.len() - 1;
        let tag = &mol.symbols[1];
        Some(vec![Molecule::new(vec![tag, &tail_len.to_string()])])
    } else {
        None
    }
}

/// lt - less than comparison
pub fn op_lt(mol: &Molecule) -> Option<Vec<Molecule>> {
    // [lt tag1 tag2 num]
    if mol.symbols.len() >= 4 {
        let tag1 = &mol.symbols[1];
        let tag2 = &mol.symbols[2];
        let num_str = &mol.symbols[3];

        if let Ok(num) = num_str.parse::<i64>() {
            // Get remaining elements
            let rest: Vec<String> = mol.symbols[4..].to_vec();
            let rest_len = rest.len() as i64;

            if rest_len < num {
                // tag1 branch
                let mut result = vec![tag1.clone()];
                result.extend(rest);
                Some(vec![Molecule::from_strings(result)])
            } else {
                // tag2 branch
                let mut result = vec![tag2.clone()];
                result.extend(rest);
                Some(vec![Molecule::from_strings(result)])
            }
        } else {
            None
        }
    } else {
        None
    }
}

/// copy - creates a copy with a new tag
pub fn op_copy(mol: &Molecule) -> Option<Vec<Molecule>> {
    if mol.symbols.len() >= 2 {
        let new_tag = &mol.symbols[1];
        let rest: Vec<String> = mol.symbols[2..].to_vec();

        let mut original = vec![mol.symbols[0].clone()];
        original.extend(rest.clone());

        let mut copied = vec![new_tag.clone()];
        copied.extend(rest);

        Some(vec![
            Molecule::from_strings(original),
            Molecule::from_strings(copied),
        ])
    } else {
        None
    }
}

/// partition - divides list into N independent molecules
pub fn op_partition(mol: &Molecule) -> Option<Vec<Molecule>> {
    // [partition N tag ...elements...]
    if mol.symbols.len() < 3 {
        return None;
    }

    let n_str = &mol.symbols[1];
    let tag = &mol.symbols[2];

    if let Ok(n_partitions) = n_str.parse::<usize>() {
        if n_partitions == 0 {
            return Some(vec![]);
        }

        let elements: Vec<String> = mol.symbols[3..].to_vec();
        let total_elements = elements.len();

        if total_elements == 0 {
            return Some(vec![]);
        }

        let base_size = total_elements / n_partitions;
        let remainder = total_elements % n_partitions;

        let mut result = Vec::new();
        let mut start_idx = 0;

        for i in 0..n_partitions {
            let partition_size = if i < remainder {
                base_size + 1
            } else {
                base_size
            };

            if partition_size == 0 {
                continue;
            }

            let end_idx = start_idx + partition_size;
            let partition_elements = &elements[start_idx..end_idx];

            let mut partition_mol = vec![tag.clone(), i.to_string()];
            partition_mol.extend_from_slice(partition_elements);

            result.push(Molecule::from_strings(partition_mol));
            start_idx = end_idx;
        }

        Some(result)
    } else {
        None
    }
}

/// merge - merges two sorted lists separated by *
pub fn op_merge(mol: &Molecule) -> Option<Vec<Molecule>> {
    // [merge tag * list1... * list2...]
    if mol.symbols.len() < 2 {
        return None;
    }

    let tag = &mol.symbols[1];
    let rest: Vec<String> = mol.symbols[2..].to_vec();

    // Find * separator
    let star_pos = rest.iter().position(|s| s == "*")?;

    // Split into two lists
    let list1: Vec<&String> = rest[..star_pos].iter().collect();
    let list2: Vec<&String> = rest[star_pos + 1..].iter().collect();

    // Merge sorted lists
    let mut merged = Vec::new();
    let mut i = 0;
    let mut j = 0;

    while i < list1.len() && j < list2.len() {
        let take_first = if is_number(list1[i]) && is_number(list2[j]) {
            let val1 = list1[i].parse::<f64>().unwrap();
            let val2 = list2[j].parse::<f64>().unwrap();
            val1 <= val2
        } else {
            list1[i] <= list2[j]
        };

        if take_first {
            merged.push(list1[i].clone());
            i += 1;
        } else {
            merged.push(list2[j].clone());
            j += 1;
        }
    }

    // Append remaining elements
    while i < list1.len() {
        merged.push(list1[i].clone());
        i += 1;
    }
    while j < list2.len() {
        merged.push(list2[j].clone());
        j += 1;
    }

    let mut result = vec![tag.clone()];
    result.extend(merged);

    Some(vec![Molecule::from_strings(result)])
}

// ============================================================================
// BIMOLECULAR OPERATIONS
// ============================================================================

pub type BimolOp = fn(&Molecule, &Molecule) -> Option<Vec<Molecule>>;

/// match - simple pattern matching (deprecated, use matchp)
pub fn op_match(mol1: &Molecule, mol2: &Molecule) -> Option<Vec<Molecule>> {
    // [match pattern] matches with [pattern ...]
    if mol1.symbols.len() < 2 || mol2.symbols.is_empty() {
        return None;
    }

    let pattern = &mol1.symbols[1];
    if mol2.symbols[0] == *pattern {
        // Match succeeds, execute tail of mol1 with tail of mol2
        let mut result = mol1.symbols[2..].to_vec();
        result.extend_from_slice(&mol2.symbols[1..]);
        Some(vec![Molecule::from_strings(result)])
    } else {
        None
    }
}

/// matchp - pattern matching with transformation
pub fn op_matchp(mol1: &Molecule, mol2: &Molecule) -> Option<Vec<Molecule>> {
    // [matchp pattern transform ...] matches with [pattern ...]
    // Returns TWO molecules: the matchp rule (to persist) and the result
    if mol1.symbols.len() < 3 || mol2.symbols.is_empty() {
        return None;
    }

    let pattern = &mol1.symbols[1];
    if mol2.symbols[0] == *pattern {
        // Match succeeds
        let transform = &mol1.symbols[2];
        let mol2_tail = &mol2.symbols[1..];
        let mol1_rest = &mol1.symbols[3..];

        // Create result: [transform mol1_rest... mol2_tail...]
        let mut result = vec![transform.clone()];
        result.extend_from_slice(mol1_rest);
        result.extend_from_slice(mol2_tail);

        // Return: [matchp rule (persists), result]
        Some(vec![
            Molecule::from_strings(mol1.symbols.clone()), // Keep the matchp rule
            Molecule::from_strings(result),                // The matched result
        ])
    } else {
        None
    }
}

// ============================================================================
// HELPERS
// ============================================================================

fn is_number(s: &str) -> bool {
    s.parse::<f64>().is_ok()
}

// ============================================================================
// OPERATION REGISTRY
// ============================================================================

pub fn get_default_rules() -> Vec<ReactionRule> {
    vec![
        ReactionRule::new("nul", "nul", op_nul),
        ReactionRule::new("pop", "pop", op_pop),
        ReactionRule::new("pop2", "pop2", op_pop2),
        ReactionRule::new("dup", "dup", op_dup),
        ReactionRule::new("exch", "exch", op_exch),
        ReactionRule::new("split", "split", op_split),
        ReactionRule::new("fork", "fork", op_fork),
        ReactionRule::new("nop", "nop", op_nop),
        ReactionRule::new("empty", "empty", op_empty),
        ReactionRule::new("length", "length", op_length),
        ReactionRule::new("lt", "lt", op_lt),
        ReactionRule::new("copy", "copy", op_copy),
        ReactionRule::new("partition", "partition", op_partition),
        ReactionRule::new("merge", "merge", op_merge),
    ]
}

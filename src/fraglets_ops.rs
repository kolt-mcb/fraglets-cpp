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

/// pop - keeps element at index 1, removes element at index 2, keeps rest
/// This is used to remove the count while preserving the tag
pub fn op_pop(mol: &Molecule) -> Option<Vec<Molecule>> {
    if mol.symbols.len() < 2 {
        return None;
    }

    if mol.symbols.len() < 4 {
        // Size 2-3: return just element at index 1
        Some(vec![Molecule::new(vec![&mol.symbols[1]])])
    } else {
        // Size >= 4: keep index 1, skip index 2, keep rest
        let mut result = vec![mol.symbols[1].clone()];
        result.extend_from_slice(&mol.symbols[3..]);
        Some(vec![Molecule::from_strings(result)])
    }
}

/// pop2 - removes elements at index 2 and 3
pub fn op_pop2(mol: &Molecule) -> Option<Vec<Molecule>> {
    if mol.symbols.len() < 3 {
        return None;
    }

    if mol.symbols.len() == 3 {
        // Return two separate molecules with index 1 and 2
        Some(vec![
            Molecule::new(vec![&mol.symbols[1]]),
            Molecule::new(vec![&mol.symbols[2]]),
        ])
    } else {
        // Size > 3: return two molecules
        // First: [index_1, index_3]
        // Second: everything from index 4 onward
        let mol1 = Molecule::new(vec![&mol.symbols[1], &mol.symbols[3]]);

        if mol.symbols.len() > 4 {
            let tail: Vec<String> = mol.symbols[4..].to_vec();
            Some(vec![mol1, Molecule::from_strings(tail)])
        } else {
            Some(vec![mol1])
        }
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

/// split - splits on first "*" delimiter into two molecules
pub fn op_split(mol: &Molecule) -> Option<Vec<Molecule>> {
    let tail = mol.tail();
    if tail.is_empty() {
        return Some(vec![]);
    }

    // Find the first "*" delimiter
    if let Some(pos) = tail.iter().position(|s| s == "*") {
        // Split into two molecules: before * and after *
        let before: Vec<String> = tail[..pos].to_vec();
        let after: Vec<String> = tail[pos+1..].to_vec();

        let mut result = Vec::new();
        if !before.is_empty() {
            result.push(Molecule::from_strings(before));
        }
        if !after.is_empty() {
            result.push(Molecule::from_strings(after));
        }
        Some(result)
    } else {
        // No delimiter found - return the whole tail as one molecule
        Some(vec![Molecule::from_strings(tail)])
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

/// empty - removes "empty" and next symbol
pub fn op_empty(mol: &Molecule) -> Option<Vec<Molecule>> {
    if mol.symbols.len() == 3 {
        // Size 3: return empty molecule (which will be ignored/disappear)
        None
    } else if mol.symbols.len() > 3 {
        // Return everything from position 2 onwards
        let result: Vec<String> = mol.symbols[2..].to_vec();
        Some(vec![Molecule::from_strings(result)])
    } else {
        // Size < 3: no reaction
        None
    }
}

/// length - returns [tag count data...] preserving the data
pub fn op_length(mol: &Molecule) -> Option<Vec<Molecule>> {
    if mol.symbols.len() <= 2 {
        return None;
    }

    // Calculate length of data (everything after "length" and "tag")
    let data_len = mol.symbols.len() - 2;
    let tag = &mol.symbols[1];
    let data = &mol.symbols[2..];

    // Build result: [tag, count, data...]
    let mut result = vec![tag.clone(), data_len.to_string()];
    result.extend_from_slice(data);

    Some(vec![Molecule::from_strings(result)])
}

/// lt - less than comparison of two numbers at index 3 and 4
pub fn op_lt(mol: &Molecule) -> Option<Vec<Molecule>> {
    // [lt tag1 tag2 num1 num2 ...]
    if mol.symbols.len() <= 4 {
        return None;
    }

    let tag1 = &mol.symbols[1];
    let tag2 = &mol.symbols[2];
    let num1_str = &mol.symbols[3];
    let num2_str = &mol.symbols[4];

    // Try to parse both as numbers
    if let (Ok(num1), Ok(num2)) = (num1_str.parse::<i64>(), num2_str.parse::<i64>()) {
        let chosen_tag = if num1 < num2 { tag1 } else { tag2 };

        // Build result: [chosen_tag, num1, num2, ...]
        let mut result = vec![chosen_tag.clone()];
        result.extend_from_slice(&mol.symbols[3..]);

        Some(vec![Molecule::from_strings(result)])
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

// Fraglets file parser (.fra format)
// Compatible with original C++ fraglets format

use crate::Molecule;
use std::fs::File;
use std::io::{BufRead, BufReader};

/// Parse a .fra file and return list of molecules
pub fn parse_fra_file(filename: &str) -> Result<Vec<Molecule>, String> {
    let file = File::open(filename)
        .map_err(|e| format!("Cannot open file {}: {}", filename, e))?;

    let reader = BufReader::new(file);
    let mut molecules = Vec::new();

    for (line_num, line) in reader.lines().enumerate() {
        let line = line.map_err(|e| format!("Error reading line {}: {}", line_num + 1, e))?;

        if let Some(mol) = parse_line(&line)? {
            molecules.push(mol);
        }
    }

    Ok(molecules)
}

/// Parse a single line from a .fra file
fn parse_line(line: &str) -> Result<Option<Molecule>, String> {
    let trimmed = line.trim();

    // Skip empty lines and comments
    if trimmed.is_empty() || trimmed.starts_with('#') {
        return Ok(None);
    }

    // Find brackets
    let start = trimmed.find('[')
        .ok_or_else(|| format!("Line missing opening bracket: {}", line))?;
    let end = trimmed.find(']')
        .ok_or_else(|| format!("Line missing closing bracket: {}", line))?;

    if end <= start {
        return Err(format!("Invalid bracket positions in line: {}", line));
    }

    // Extract content between brackets
    let content = &trimmed[start + 1..end];

    // Split into symbols
    let symbols: Vec<&str> = content.split_whitespace().collect();

    if symbols.is_empty() {
        return Ok(None); // Empty molecule, skip
    }

    Ok(Some(Molecule::new(symbols)))
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_parse_simple_molecule() {
        let line = " [nul] ";
        let mol = parse_line(line).unwrap().unwrap();
        assert_eq!(mol.symbols, vec!["nul"]);
    }

    #[test]
    fn test_parse_molecule_with_data() {
        let line = "[matchp sort empty finish continue]";
        let mol = parse_line(line).unwrap().unwrap();
        assert_eq!(mol.symbols, vec!["matchp", "sort", "empty", "finish", "continue"]);
    }

    #[test]
    fn test_parse_numbers() {
        let line = "[sort 203 -200 989 -446]";
        let mol = parse_line(line).unwrap().unwrap();
        assert_eq!(mol.symbols, vec!["sort", "203", "-200", "989", "-446"]);
    }

    #[test]
    fn test_skip_comment() {
        let line = "# this is a comment";
        let mol = parse_line(line).unwrap();
        assert!(mol.is_none());
    }

    #[test]
    fn test_skip_empty() {
        let line = "  ";
        let mol = parse_line(line).unwrap();
        assert!(mol.is_none());
    }
}

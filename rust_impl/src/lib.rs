// Spatial Fraglets - Lock-Free Parallel Chemical Computing
// Core implementation

use crossbeam_channel::{bounded, Sender, Receiver};
use std::thread;
use rand::Rng;

// Submodules
pub mod fraglets_ops;
pub mod parser;
pub mod bimol_region;
pub mod fraglets_system;

// Re-exports for convenience
pub use fraglets_ops::{get_default_rules, op_matchp, op_match, BimolOp};
pub use parser::parse_fra_file;
pub use fraglets_system::{FragletsSystem, CompleteFragletsBuilder};
pub use bimol_region::{BimolRegion, BimolReactionRule};

// ============================================================================
// MOLECULE
// ============================================================================

#[derive(Clone, Debug, PartialEq)]
pub struct Molecule {
    pub symbols: Vec<String>,
}

impl Molecule {
    pub fn new(symbols: Vec<&str>) -> Self {
        Molecule {
            symbols: symbols.iter().map(|s| s.to_string()).collect(),
        }
    }

    pub fn from_strings(symbols: Vec<String>) -> Self {
        Molecule { symbols }
    }

    pub fn head(&self) -> Option<&str> {
        self.symbols.first().map(|s| s.as_str())
    }

    pub fn tail(&self) -> Vec<String> {
        if self.symbols.len() > 1 {
            self.symbols[1..].to_vec()
        } else {
            vec![]
        }
    }

    pub fn matches_pattern(&self, pattern: &str) -> bool {
        self.head() == Some(pattern)
    }
}

// ============================================================================
// REACTION RULES
// ============================================================================

pub type ReactionFn = fn(&Molecule) -> Option<Vec<Molecule>>;

pub struct ReactionRule {
    pub name: String,
    pub pattern: String,
    pub action: ReactionFn,
}

impl Clone for ReactionRule {
    fn clone(&self) -> Self {
        ReactionRule {
            name: self.name.clone(),
            pattern: self.pattern.clone(),
            action: self.action, // fn pointers are Copy
        }
    }
}

impl ReactionRule {
    pub fn new(name: &str, pattern: &str, action: ReactionFn) -> Self {
        ReactionRule {
            name: name.to_string(),
            pattern: pattern.to_string(),
            action,
        }
    }

    pub fn apply(&self, mol: &Molecule) -> Option<Vec<Molecule>> {
        if mol.matches_pattern(&self.pattern) {
            (self.action)(mol)
        } else {
            None
        }
    }
}

// ============================================================================
// REGION - Thread-Local Reactor
// ============================================================================

pub struct Region {
    pub id: usize,
    pub molecules: Vec<Molecule>,
    pub rules: Vec<ReactionRule>,
    pub inbox: Receiver<Molecule>,
    pub outboxes: Vec<Sender<Molecule>>,
    pub reactions_processed: usize,
    pub diffusion_rate: f64,
}

impl Region {
    pub fn new(
        id: usize,
        inbox: Receiver<Molecule>,
        outboxes: Vec<Sender<Molecule>>,
        diffusion_rate: f64,
    ) -> Self {
        Region {
            id,
            molecules: Vec::new(),
            rules: Vec::new(),
            inbox,
            outboxes,
            reactions_processed: 0,
            diffusion_rate,
        }
    }

    /// Main execution step - FULLY PARALLEL with other regions!
    pub fn step(&mut self) -> bool {
        // 1. Receive migrating molecules (non-blocking)
        let mut received = 0;
        while let Ok(mol) = self.inbox.try_recv() {
            self.molecules.push(mol);
            received += 1;
        }

        // 2. Process reactions (NO LOCKS NEEDED!)
        let reacted = self.react();

        // 3. Simulate diffusion
        self.diffuse();

        // Continue if we did any work or have molecules
        reacted > 0 || received > 0 || !self.molecules.is_empty()
    }

    /// Process all possible reactions
    fn react(&mut self) -> usize {
        let mut reactions = 0;
        let mut i = 0;

        while i < self.molecules.len() {
            let mol = &self.molecules[i];
            let mut reacted = false;

            // Try each rule
            for rule in &self.rules {
                if let Some(products) = rule.apply(mol) {
                    // Remove reactant
                    self.molecules.swap_remove(i);

                    // Add products
                    self.molecules.extend(products);

                    self.reactions_processed += 1;
                    reactions += 1;
                    reacted = true;
                    break;
                }
            }

            if !reacted {
                i += 1;
            }
        }

        reactions
    }

    /// Simulate molecular diffusion to neighbors
    fn diffuse(&mut self) {
        if self.outboxes.is_empty() {
            return;
        }

        let mut rng = rand::thread_rng();
        let mut migrants = Vec::new();

        // Collect molecules to migrate
        for i in (0..self.molecules.len()).rev() {
            if rng.gen::<f64>() < self.diffusion_rate {
                migrants.push(self.molecules.swap_remove(i));
            }
        }

        // Send to random neighbors
        for mol in migrants {
            let neighbor = rng.gen_range(0..self.outboxes.len());
            let _ = self.outboxes[neighbor].send(mol);
        }
    }

    pub fn add_rule(&mut self, rule: ReactionRule) {
        self.rules.push(rule);
    }

    pub fn add_molecule(&mut self, mol: Molecule) {
        self.molecules.push(mol);
    }
}

// ============================================================================
// SPATIAL FRAGLETS SYSTEM
// ============================================================================

pub struct SpatialFraglets {
    pub num_regions: usize,
    pub diffusion_rate: f64,
}

impl SpatialFraglets {
    pub fn new(num_regions: usize) -> Self {
        SpatialFraglets {
            num_regions,
            diffusion_rate: 0.05, // 5% migrate per step
        }
    }

    pub fn with_diffusion(num_regions: usize, diffusion_rate: f64) -> Self {
        SpatialFraglets {
            num_regions,
            diffusion_rate,
        }
    }

    /// Run the fraglets system with given initial molecules and rules
    pub fn run(
        &self,
        initial_molecules: Vec<Molecule>,
        rules: Vec<ReactionRule>,
        max_iterations: usize,
    ) -> RunResult {
        let start = std::time::Instant::now();

        // Create channels for each region
        let mut channels = Vec::new();
        for _ in 0..self.num_regions {
            channels.push(bounded(1000)); // Buffered channels
        }

        let senders: Vec<_> = channels.iter().map(|(s, _)| s.clone()).collect();

        // Distribute molecules across regions
        let mut region_molecules: Vec<Vec<Molecule>> = vec![Vec::new(); self.num_regions];
        for (i, mol) in initial_molecules.into_iter().enumerate() {
            region_molecules[i % self.num_regions].push(mol);
        }

        // Spawn worker threads
        let mut handles = Vec::new();

        for (region_id, (_sender, receiver)) in channels.into_iter().enumerate() {
            let outboxes = senders.clone();
            let molecules = region_molecules.remove(0);
            let rules = rules.clone();
            let diffusion_rate = self.diffusion_rate;

            let handle = thread::spawn(move || {
                let mut region = Region::new(region_id, receiver, outboxes, diffusion_rate);

                // Initialize
                region.molecules = molecules;
                for rule in rules {
                    region.add_rule(rule);
                }

                // Main loop
                for _iteration in 0..max_iterations {
                    let active = region.step();

                    if !active && region.molecules.is_empty() {
                        break;
                    }
                }

                RegionResult {
                    id: region.id,
                    reactions: region.reactions_processed,
                    remaining_molecules: region.molecules,
                }
            });

            handles.push(handle);
        }

        // Wait for completion
        let mut results = Vec::new();
        for handle in handles {
            results.push(handle.join().unwrap());
        }

        let duration = start.elapsed();

        RunResult {
            duration,
            regions: results,
        }
    }
}

// ============================================================================
// RESULTS
// ============================================================================

#[derive(Debug)]
pub struct RegionResult {
    pub id: usize,
    pub reactions: usize,
    pub remaining_molecules: Vec<Molecule>,
}

#[derive(Debug)]
pub struct RunResult {
    pub duration: std::time::Duration,
    pub regions: Vec<RegionResult>,
}

impl RunResult {
    pub fn total_reactions(&self) -> usize {
        self.regions.iter().map(|r| r.reactions).sum()
    }

    pub fn total_molecules(&self) -> usize {
        self.regions.iter().map(|r| r.remaining_molecules.len()).sum()
    }

    pub fn collect_molecules(&self) -> Vec<Molecule> {
        self.regions
            .iter()
            .flat_map(|r| r.remaining_molecules.clone())
            .collect()
    }
}

// ============================================================================
// COMMON REACTIONS
// ============================================================================

/// null operation - molecule disappears
pub fn nul(_mol: &Molecule) -> Option<Vec<Molecule>> {
    Some(vec![]) // Disappears
}

/// duplicate - creates a copy
pub fn dup(mol: &Molecule) -> Option<Vec<Molecule>> {
    let tail = mol.tail();
    if !tail.is_empty() {
        Some(vec![
            Molecule::from_strings(tail.clone()),
            Molecule::from_strings(tail),
        ])
    } else {
        None
    }
}

/// split - breaks into individual symbols
pub fn split(mol: &Molecule) -> Option<Vec<Molecule>> {
    let tail = mol.tail();
    if !tail.is_empty() {
        Some(tail.into_iter().map(|s| Molecule::new(vec![&s])).collect())
    } else {
        None
    }
}

// ============================================================================
// BUILDER FOR CONVENIENCE
// ============================================================================

pub struct FragletsBuilder {
    molecules: Vec<Molecule>,
    rules: Vec<ReactionRule>,
    num_regions: usize,
    diffusion_rate: f64,
}

impl FragletsBuilder {
    pub fn new() -> Self {
        FragletsBuilder {
            molecules: Vec::new(),
            rules: Vec::new(),
            num_regions: 4,
            diffusion_rate: 0.05,
        }
    }

    pub fn regions(mut self, n: usize) -> Self {
        self.num_regions = n;
        self
    }

    pub fn diffusion(mut self, rate: f64) -> Self {
        self.diffusion_rate = rate;
        self
    }

    pub fn add_molecule(mut self, mol: Molecule) -> Self {
        self.molecules.push(mol);
        self
    }

    pub fn add_rule(mut self, rule: ReactionRule) -> Self {
        self.rules.push(rule);
        self
    }

    pub fn run(self, max_iterations: usize) -> RunResult {
        let system = SpatialFraglets::with_diffusion(self.num_regions, self.diffusion_rate);
        system.run(self.molecules, self.rules, max_iterations)
    }
}

impl Default for FragletsBuilder {
    fn default() -> Self {
        Self::new()
    }
}

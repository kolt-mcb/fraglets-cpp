// Extended Region with bimolecular operation support

use crate::{Molecule, ReactionRule, BimolOp, ReactionEvent, ReactionType};
use crossbeam_channel::{Sender, Receiver};
use rand::Rng;
use std::sync::Arc;
use std::collections::hash_map::DefaultHasher;
use std::hash::{Hash, Hasher};

pub struct BimolReactionRule {
    pub name: String,
    pub pattern: String,
    pub action: BimolOp,
}

impl BimolReactionRule {
    pub fn new(name: &str, pattern: &str, action: BimolOp) -> Self {
        BimolReactionRule {
            name: name.to_string(),
            pattern: pattern.to_string(),
            action,
        }
    }

    pub fn apply(&self, mol1: &Molecule, mol2: &Molecule) -> Option<Vec<Molecule>> {
        // Check if mol1 starts with pattern
        if mol1.head() == Some(self.pattern.as_str()) {
            (self.action)(mol1, mol2)
        } else {
            None
        }
    }
}

pub struct BimolRegion {
    pub id: usize,
    pub molecules: Vec<Molecule>,
    pub unimol_rules: Vec<ReactionRule>,
    pub bimol_rules: Vec<BimolReactionRule>,
    pub persistent_matchp: Arc<Vec<Molecule>>,  // Shared across all regions
    pub inbox: Receiver<Molecule>,
    pub outboxes: Vec<Sender<Molecule>>,
    pub reactions_processed: usize,
    pub diffusion_rate: f64,
    pub reaction_history: Vec<ReactionEvent>,
    pub use_pattern_routing: bool,  // Enable pattern-based routing
}

impl BimolRegion {
    pub fn new(
        id: usize,
        inbox: Receiver<Molecule>,
        outboxes: Vec<Sender<Molecule>>,
        diffusion_rate: f64,
        persistent_matchp: Arc<Vec<Molecule>>,
        use_pattern_routing: bool,
    ) -> Self {
        BimolRegion {
            id,
            molecules: Vec::new(),
            unimol_rules: Vec::new(),
            bimol_rules: Vec::new(),
            persistent_matchp,
            inbox,
            outboxes,
            reactions_processed: 0,
            diffusion_rate,
            reaction_history: Vec::new(),
            use_pattern_routing,
        }
    }

    pub fn step(&mut self) -> bool {
        // 1. Receive migrating molecules
        let mut received = 0;
        while let Ok(mol) = self.inbox.try_recv() {
            self.molecules.push(mol);
            received += 1;
        }

        // 2. Process reactions
        let reacted = self.react_unimol() + self.react_bimol() + self.react_persistent_matchp();

        // 3. Simulate diffusion or routing
        if self.use_pattern_routing {
            self.route_molecules();
        } else {
            self.diffuse();
        }

        // Continue if we did any work or have molecules
        reacted > 0 || received > 0 || !self.molecules.is_empty()
    }

    fn react_unimol(&mut self) -> usize {
        let mut reactions = 0;
        let mut i = 0;

        while i < self.molecules.len() {
            let mol = &self.molecules[i];
            let mut reacted = false;

            for rule in &self.unimol_rules {
                if let Some(products) = rule.apply(mol) {
                    let reactant = mol.clone();
                    self.molecules.swap_remove(i);
                    self.molecules.extend(products.clone());

                    // Record reaction
                    self.reaction_history.push(ReactionEvent {
                        reactants: vec![reactant],
                        products,
                        reaction_type: ReactionType::Unimol,
                        region_id: self.id,
                    });

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

    fn react_bimol(&mut self) -> usize {
        let mut reactions = 0;
        let mut i = 0;

        while i < self.molecules.len() {
            let mol1 = &self.molecules[i];
            let mut matched = false;

            // Try to find a matching partner
            for rule in &self.bimol_rules {
                // Check if mol1 matches the bimol pattern
                if mol1.head() == Some(rule.pattern.as_str()) {
                    // Look for a partner molecule
                    if let Some((j, products)) = self.find_bimol_partner(i, rule) {
                        // Save reactants before removing
                        let reactant1 = self.molecules[i].clone();
                        let reactant2 = self.molecules[j].clone();
                        let is_matchp = rule.pattern == "matchp";

                        // Remove both molecules (remove higher index first)
                        let (idx1, idx2) = if i < j { (j, i) } else { (i, j) };
                        self.molecules.swap_remove(idx1);
                        self.molecules.swap_remove(idx2);

                        // Add products
                        self.molecules.extend(products.clone());

                        // Record reaction
                        self.reaction_history.push(ReactionEvent {
                            reactants: vec![reactant1, reactant2],
                            products,
                            reaction_type: if is_matchp { ReactionType::Matchp } else { ReactionType::Bimol },
                            region_id: self.id,
                        });

                        self.reactions_processed += 1;
                        reactions += 1;
                        matched = true;
                        break;
                    }
                }
            }

            if !matched {
                i += 1;
            }
        }

        reactions
    }

    fn find_bimol_partner(&self, active_idx: usize, rule: &BimolReactionRule) -> Option<(usize, Vec<Molecule>)> {
        let active_mol = &self.molecules[active_idx];

        // Try to match with any other molecule
        for (j, passive_mol) in self.molecules.iter().enumerate() {
            if j == active_idx {
                continue;
            }

            if let Some(products) = rule.apply(active_mol, passive_mol) {
                return Some((j, products));
            }
        }

        None
    }

    fn diffuse(&mut self) {
        if self.outboxes.is_empty() {
            return;
        }

        let mut rng = rand::thread_rng();
        let mut migrants = Vec::new();

        for i in (0..self.molecules.len()).rev() {
            if rng.gen::<f64>() < self.diffusion_rate {
                migrants.push(self.molecules.swap_remove(i));
            }
        }

        for mol in migrants {
            let neighbor = rng.gen_range(0..self.outboxes.len());
            // Use try_send to avoid blocking - if channel is full, keep the molecule
            if self.outboxes[neighbor].try_send(mol.clone()).is_err() {
                self.molecules.push(mol);
            }
        }
    }

    /// React local molecules against shared persistent matchp rules
    fn react_persistent_matchp(&mut self) -> usize {
        use crate::fraglets_ops::op_matchp;

        let mut reactions = 0;
        let mut i = 0;

        while i < self.molecules.len() {
            let mol = &self.molecules[i];
            let mut matched = false;

            // Try to match against each persistent matchp rule
            for matchp_rule in self.persistent_matchp.iter() {
                if matchp_rule.head() != Some("matchp") {
                    continue;
                }

                // Check if this matchp can react with mol
                if let Some(products) = op_matchp(matchp_rule, mol) {
                    // Save reactants
                    let reactant1 = matchp_rule.clone();
                    let reactant2 = mol.clone();

                    // Remove the data molecule (matchp rule persists via shared Arc)
                    self.molecules.swap_remove(i);

                    // op_matchp returns [matchp_rule, result]
                    // We only add the result since matchp_rule is already in persistent_matchp
                    if products.len() >= 2 {
                        // Add only the result (skip the first element which is the matchp rule)
                        self.molecules.extend(products[1..].iter().cloned());
                    }

                    // Record reaction with full products for history
                    self.reaction_history.push(ReactionEvent {
                        reactants: vec![reactant1, reactant2],
                        products,
                        reaction_type: ReactionType::Matchp,
                        region_id: self.id,
                    });

                    self.reactions_processed += 1;
                    reactions += 1;
                    matched = true;
                    break;
                }
            }

            if !matched {
                i += 1;
            }
        }

        reactions
    }

    /// Route molecules to appropriate regions based on head pattern
    fn route_molecules(&mut self) {
        if self.outboxes.is_empty() {
            return;
        }

        let num_regions = self.outboxes.len();
        let mut to_route = Vec::new();

        // Collect molecules that should be routed
        for i in (0..self.molecules.len()).rev() {
            let mol = &self.molecules[i];

            // Skip matchp rules - they stay in shared Arc
            if mol.head() == Some("matchp") {
                continue;
            }

            // Calculate target region based on head pattern
            let target_region = Self::hash_pattern(mol.head().unwrap_or("")) % num_regions;

            // If not in correct region, route it
            if target_region != self.id {
                to_route.push((target_region, self.molecules.swap_remove(i)));
            }
        }

        // Send molecules to target regions
        for (target, mol) in to_route {
            if self.outboxes[target].try_send(mol.clone()).is_err() {
                // If send fails, keep the molecule locally
                self.molecules.push(mol);
            }
        }
    }

    /// Hash a pattern string to determine target region
    fn hash_pattern(pattern: &str) -> usize {
        let mut hasher = DefaultHasher::new();
        pattern.hash(&mut hasher);
        hasher.finish() as usize
    }
}

// Extended Region with bimolecular operation support

use crate::{Molecule, ReactionRule, BimolOp};
use crossbeam_channel::{Sender, Receiver};
use rand::Rng;

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
    pub inbox: Receiver<Molecule>,
    pub outboxes: Vec<Sender<Molecule>>,
    pub reactions_processed: usize,
    pub diffusion_rate: f64,
}

impl BimolRegion {
    pub fn new(
        id: usize,
        inbox: Receiver<Molecule>,
        outboxes: Vec<Sender<Molecule>>,
        diffusion_rate: f64,
    ) -> Self {
        BimolRegion {
            id,
            molecules: Vec::new(),
            unimol_rules: Vec::new(),
            bimol_rules: Vec::new(),
            inbox,
            outboxes,
            reactions_processed: 0,
            diffusion_rate,
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
        let reacted = self.react_unimol() + self.react_bimol();

        // 3. Simulate diffusion
        self.diffuse();

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
                    self.molecules.swap_remove(i);
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
                        // Remove both molecules (remove higher index first)
                        let (idx1, idx2) = if i < j { (j, i) } else { (i, j) };
                        self.molecules.swap_remove(idx1);
                        self.molecules.swap_remove(idx2);

                        // Add products
                        self.molecules.extend(products);

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
            let _ = self.outboxes[neighbor].send(mol);
        }
    }
}

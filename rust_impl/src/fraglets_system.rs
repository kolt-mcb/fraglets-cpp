// Complete fraglets system with bimolecular support

use crate::{Molecule, RunResult, RegionResult};
use crate::bimol_region::{BimolRegion, BimolReactionRule};
use crate::fraglets_ops::{op_match, op_matchp};
use crate::ReactionRule;
use crossbeam_channel::bounded;
use std::thread;

pub struct FragletsSystem {
    pub num_regions: usize,
    pub diffusion_rate: f64,
}

impl FragletsSystem {
    pub fn new(num_regions: usize) -> Self {
        FragletsSystem {
            num_regions,
            diffusion_rate: 0.05,
        }
    }

    pub fn with_diffusion(num_regions: usize, diffusion_rate: f64) -> Self {
        FragletsSystem {
            num_regions,
            diffusion_rate,
        }
    }

    pub fn run(
        &self,
        initial_molecules: Vec<Molecule>,
        unimol_rules: Vec<ReactionRule>,
        bimol_rules: Vec<BimolReactionRule>,
        max_iterations: usize,
    ) -> RunResult {
        let start = std::time::Instant::now();

        // Create channels
        let mut channels = Vec::new();
        for _ in 0..self.num_regions {
            channels.push(bounded(1000));
        }

        let senders: Vec<_> = channels.iter().map(|(s, _)| s.clone()).collect();

        // Distribute molecules
        let mut region_molecules: Vec<Vec<Molecule>> = vec![Vec::new(); self.num_regions];
        for (i, mol) in initial_molecules.into_iter().enumerate() {
            region_molecules[i % self.num_regions].push(mol);
        }

        // Spawn workers
        let mut handles = Vec::new();

        for (region_id, (_sender, receiver)) in channels.into_iter().enumerate() {
            let outboxes = senders.clone();
            let molecules = region_molecules.remove(0);
            let unimol_rules = unimol_rules.clone();
            let bimol_rules = bimol_rules.clone();
            let diffusion_rate = self.diffusion_rate;

            let handle = thread::spawn(move || {
                let mut region = BimolRegion::new(region_id, receiver, outboxes, diffusion_rate);

                region.molecules = molecules;
                region.unimol_rules = unimol_rules;
                region.bimol_rules = bimol_rules;

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

        // Wait
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

pub struct CompleteFragletsBuilder {
    molecules: Vec<Molecule>,
    unimol_rules: Vec<ReactionRule>,
    bimol_rules: Vec<BimolReactionRule>,
    num_regions: usize,
    diffusion_rate: f64,
}

impl CompleteFragletsBuilder {
    pub fn new() -> Self {
        // Add default bimol rules
        let bimol_rules = vec![
            BimolReactionRule::new("match", "match", op_match),
            BimolReactionRule::new("matchp", "matchp", op_matchp),
        ];

        CompleteFragletsBuilder {
            molecules: Vec::new(),
            unimol_rules: Vec::new(),
            bimol_rules,
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

    pub fn add_molecules(mut self, mols: Vec<Molecule>) -> Self {
        self.molecules.extend(mols);
        self
    }

    pub fn add_unimol_rule(mut self, rule: ReactionRule) -> Self {
        self.unimol_rules.push(rule);
        self
    }

    pub fn add_bimol_rule(mut self, rule: BimolReactionRule) -> Self {
        self.bimol_rules.push(rule);
        self
    }

    pub fn run(self, max_iterations: usize) -> RunResult {
        let system = FragletsSystem::with_diffusion(self.num_regions, self.diffusion_rate);
        system.run(self.molecules, self.unimol_rules, self.bimol_rules, max_iterations)
    }
}

impl Default for CompleteFragletsBuilder {
    fn default() -> Self {
        Self::new()
    }
}

impl Clone for BimolReactionRule {
    fn clone(&self) -> Self {
        BimolReactionRule {
            name: self.name.clone(),
            pattern: self.pattern.clone(),
            action: self.action,
        }
    }
}

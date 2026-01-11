// Shared Pool implementation - eliminates regions
//
// Key idea: Use multiplicity-aware fine-grained locking instead of spatial partitioning

use crate::{Molecule, ReactionRule};
use dashmap::DashMap;
use std::sync::Arc;
use std::sync::atomic::{AtomicUsize, Ordering};
use rand::Rng;

pub struct SharedPool {
    // Molecules grouped by head symbol with concurrent access
    molecules: DashMap<String, Vec<Molecule>>,

    // Persistent matchp rules (shared, immutable)
    matchp_rules: Arc<Vec<Molecule>>,

    // Unimolecular and bimolecular rules
    unimol_rules: Vec<ReactionRule>,

    // Total reactions processed
    reactions: AtomicUsize,

    // Number of worker threads
    num_workers: usize,
}

impl SharedPool {
    pub fn new(num_workers: usize) -> Self {
        SharedPool {
            molecules: DashMap::new(),
            matchp_rules: Arc::new(Vec::new()),
            unimol_rules: Vec::new(),
            reactions: AtomicUsize::new(0),
            num_workers,
        }
    }

    pub fn with_matchp_rules(mut self, rules: Vec<Molecule>) -> Self {
        self.matchp_rules = Arc::new(rules);
        self
    }

    pub fn add_unimol_rule(&mut self, rule: ReactionRule) {
        self.unimol_rules.push(rule);
    }

    /// Insert a molecule into the pool
    /// Lock is held only for the duration of the push (~nanoseconds)
    pub fn insert(&self, mol: Molecule) {
        if let Some(head) = mol.head() {
            let key = head.to_string();
            self.molecules.entry(key).or_insert(Vec::new()).push(mol);
        }
    }

    /// Insert multiple molecules
    pub fn insert_many(&self, molecules: Vec<Molecule>) {
        for mol in molecules {
            self.insert(mol);
        }
    }

    /// Try to pop a molecule from a specific key
    /// Lock is held only for the duration of the pop (~nanoseconds)
    pub fn try_pop(&self, key: &str) -> Option<Molecule> {
        self.molecules.get_mut(key).and_then(|mut vec| vec.pop())
    }

    /// Get multiplicity for a specific key (lock-free read)
    pub fn multiplicity(&self, key: &str) -> usize {
        self.molecules.get(key).map(|v| v.len()).unwrap_or(0)
    }

    /// Get all keys with their multiplicities (lock-free reads)
    pub fn get_key_multiplicities(&self) -> Vec<(String, usize)> {
        self.molecules
            .iter()
            .map(|entry| (entry.key().clone(), entry.value().len()))
            .filter(|(_, count)| *count > 0)
            .collect()
    }

    /// Total number of molecules across all keys
    pub fn total_molecules(&self) -> usize {
        self.molecules.iter().map(|e| e.value().len()).sum()
    }

    /// Determine optimal number of workers based on multiplicity
    pub fn optimal_workers(&self) -> usize {
        let total = self.total_molecules();
        let num_keys = self.molecules.len();

        if num_keys == 0 || total == 0 {
            return 1;
        }

        let avg_multiplicity = total / num_keys;

        // Use multiplicity insight from benchmarks
        // M/R >= 8-16 is the sweet spot
        let optimal = match avg_multiplicity {
            0..=7   => 1,   // Too low, single thread
            8..=15  => 2,   // Marginal benefit
            16..=31 => 4,   // Good benefit
            32..=63 => 6,   // Better benefit
            _       => 8,   // Excellent benefit
        };

        // Don't exceed configured workers
        optimal.min(self.num_workers)
    }

    /// Select a key to work on, preferring high-multiplicity keys
    /// This minimizes contention by avoiding low-multiplicity keys
    fn select_key(&self) -> Option<String> {
        let keys = self.get_key_multiplicities();

        if keys.is_empty() {
            return None;
        }

        // Prefer high-multiplicity keys (lower contention)
        // Use weighted random selection based on multiplicity
        let total_weight: usize = keys.iter().map(|(_, m)| m).sum();

        if total_weight == 0 {
            return None;
        }

        let mut rng = rand::thread_rng();
        let mut threshold = rng.gen_range(0..total_weight);

        for (key, mult) in keys {
            if threshold < mult {
                return Some(key);
            }
            threshold -= mult;
        }

        // Fallback to any key
        self.molecules.iter().next().map(|e| e.key().clone())
    }

    /// Try to perform one reaction
    /// Returns true if a reaction occurred
    pub fn try_react(&self) -> bool {
        // Try unimolecular reaction first (simpler)
        if let Some(key) = self.select_key() {
            if let Some(mol) = self.try_pop(&key) {
                // Check if it matches any unimolecular rule
                for rule in &self.unimol_rules {
                    if let Some(products) = rule.apply(&mol) {
                        self.insert_many(products);
                        self.reactions.fetch_add(1, Ordering::Relaxed);
                        return true;
                    }
                }

                // Try bimolecular with persistent matchp
                for matchp_mol in self.matchp_rules.iter() {
                    if let Some(head) = matchp_mol.head() {
                        if head == "matchp" {
                            if let Some(products) = crate::op_matchp(matchp_mol, &mol) {
                                self.insert_many(products);
                                self.reactions.fetch_add(1, Ordering::Relaxed);
                                return true;
                            }
                        }
                    }
                }

                // No reaction, put it back
                self.insert(mol);
            }
        }

        false
    }

    /// Run reactions until idle or max_iterations reached
    pub fn run(self: Arc<Self>, max_iterations: usize) -> usize {
        let num_workers = self.optimal_workers();

        if num_workers == 1 {
            // Single-threaded execution
            return self.run_single_threaded(max_iterations);
        }

        // Multi-threaded execution
        self.run_multi_threaded(max_iterations, num_workers)
    }

    fn run_single_threaded(&self, max_iterations: usize) -> usize {
        let mut iterations = 0;

        while iterations < max_iterations && self.total_molecules() > 0 {
            if !self.try_react() {
                break;  // Idle
            }
            iterations += 1;
        }

        self.reactions.load(Ordering::Relaxed)
    }

    fn run_multi_threaded(self: Arc<Self>, max_iterations: usize, num_workers: usize) -> usize {
        use std::sync::atomic::AtomicBool;
        use std::thread;

        let done = Arc::new(AtomicBool::new(false));
        let iter_count = Arc::new(AtomicUsize::new(0));

        let handles: Vec<_> = (0..num_workers)
            .map(|_worker_id| {
                let pool = Arc::clone(&self);
                let done = Arc::clone(&done);
                let iter_count = Arc::clone(&iter_count);

                thread::spawn(move || {
                    while !done.load(Ordering::Relaxed) {
                        // Check iteration limit
                        let current_iter = iter_count.load(Ordering::Relaxed);
                        if current_iter >= max_iterations {
                            break;
                        }

                        // Try to react
                        if pool.try_react() {
                            iter_count.fetch_add(1, Ordering::Relaxed);
                        } else {
                            // Back off if idle
                            thread::yield_now();
                        }

                        // Check if pool is empty
                        if pool.total_molecules() == 0 {
                            break;
                        }
                    }
                })
            })
            .collect();

        // Wait for all workers
        for handle in handles {
            handle.join().unwrap();
        }

        done.store(true, Ordering::Relaxed);

        self.reactions.load(Ordering::Relaxed)
    }

    /// Collect all remaining molecules
    pub fn collect_molecules(&self) -> Vec<Molecule> {
        let mut all_molecules = Vec::new();

        for entry in self.molecules.iter() {
            all_molecules.extend(entry.value().clone());
        }

        all_molecules
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_multiplicity_tracking() {
        let pool = SharedPool::new(4);

        pool.insert(Molecule::new(vec!["work", "1"]));
        pool.insert(Molecule::new(vec!["work", "2"]));
        pool.insert(Molecule::new(vec!["work", "3"]));

        assert_eq!(pool.multiplicity("work"), 3);
        assert_eq!(pool.total_molecules(), 3);
    }

    #[test]
    fn test_optimal_workers() {
        let pool = SharedPool::new(8);

        // Low multiplicity - should recommend 1 worker
        for i in 0..3 {
            pool.insert(Molecule::new(vec!["work", &i.to_string()]));
        }
        assert_eq!(pool.optimal_workers(), 1);

        // High multiplicity - should recommend more workers
        for i in 3..100 {
            pool.insert(Molecule::new(vec!["work", &i.to_string()]));
        }
        assert!(pool.optimal_workers() >= 4);
    }

    #[test]
    fn test_concurrent_access() {
        use std::thread;

        let pool = Arc::new(SharedPool::new(4));

        // Insert 100 molecules from multiple threads
        let handles: Vec<_> = (0..4)
            .map(|thread_id| {
                let pool = Arc::clone(&pool);
                thread::spawn(move || {
                    for i in 0..25 {
                        pool.insert(Molecule::new(vec!["work", &(thread_id * 25 + i).to_string()]));
                    }
                })
            })
            .collect();

        for handle in handles {
            handle.join().unwrap();
        }

        assert_eq!(pool.total_molecules(), 100);
        assert_eq!(pool.multiplicity("work"), 100);
    }
}

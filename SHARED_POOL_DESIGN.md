# Shared Pool Design - Eliminating Regions

## Problem with Current Regions Approach

The spatial partitioning design has several inefficiencies:

1. **Channel Overhead**: Molecules must be sent/received via crossbeam channels
2. **Routing Overhead**: Pattern hashing or round-robin distribution
3. **Uneven Distribution**: Random distribution can starve some regions
4. **Memory Duplication**: Each region has separate Vec<Molecule>
5. **Complexity**: Inbox/outbox management, diffusion simulation

## New Design: Multiplicity-Aware Shared Pool

### Key Insight

**When multiplicity >> num_threads, lock contention is minimal.**

From benchmark results:
- M/R = 8-16: Low contention, good performance
- M/R = 32+: Minimal contention, excellent performance

Instead of avoiding contention with regions, **embrace it with fine-grained locks when multiplicity is high.**

### Architecture

```rust
pub struct SharedMoleculePool {
    // Molecules grouped by head symbol (like keyMultiset in C++)
    molecules: HashMap<String, Mutex<Vec<Molecule>>>,

    // Persistent matchp rules (shared, immutable)
    matchp_rules: Arc<Vec<Molecule>>,

    // Multiplicity tracking for dynamic scheduling
    multiplicities: DashMap<String, AtomicUsize>,

    // Worker threads
    num_workers: usize,
}
```

### Per-Key Locking Strategy

Instead of one global lock:
```rust
// BAD: Global lock (high contention)
let molecules = Mutex::new(Vec<Molecule>);

// GOOD: Per-key locks (low contention)
let molecules = HashMap<String, Mutex<Vec<Molecule>>>;
```

**Why this works:**
- Different patterns can react in parallel (different locks)
- Same pattern with high multiplicity: short critical section (just pop)
- Lock is only held during molecule removal (~nanoseconds)

### Lock-Free Multiplicity Tracking

```rust
use dashmap::DashMap;  // Concurrent HashMap

pub struct SharedPool {
    molecules: DashMap<String, Vec<Molecule>>,
    matchp_rules: Arc<Vec<Molecule>>,
}
```

**DashMap** provides:
- Lock-free reads for checking multiplicity
- Fine-grained internal locking for modifications
- Much lower overhead than channels

### Worker Thread Design

```rust
fn worker_thread(pool: Arc<SharedPool>, worker_id: usize) {
    loop {
        // 1. Check multiplicities (lock-free read)
        let available_keys = pool.get_available_keys();

        // 2. Pick a key to work on
        let key = select_key_by_multiplicity(&available_keys);

        // 3. Try to grab a molecule (short lock)
        if let Some(mol) = pool.try_pop(&key) {
            // 4. React (outside lock)
            let products = react(mol, &pool.matchp_rules);

            // 5. Insert products (short lock per product)
            pool.insert_many(products);
        }
    }
}
```

**Critical Observation:**
- Lock held only during `pop()` and `insert()` (microseconds)
- Actual reaction computation is lock-free
- With M/R = 16+, collisions are rare

### Dynamic Multiplicity Scheduling

```rust
fn select_key_by_multiplicity(keys: &[(String, usize)]) -> String {
    // Prefer high-multiplicity keys to minimize contention
    keys.iter()
        .max_by_key(|(_, mult)| mult)
        .map(|(k, _)| k.clone())
        .unwrap()
}
```

When multiplicity is low, workers naturally converge on high-multiplicity keys, avoiding contention on scarce resources.

## Comparison: Regions vs Shared Pool

| Aspect | Regions | Shared Pool |
|--------|---------|-------------|
| **Synchronization** | Channels | Fine-grained locks |
| **Distribution** | Random (uneven) | Pull-based (even) |
| **Overhead** | Channel send/recv | Lock acquire/release |
| **Memory** | N copies | 1 shared pool |
| **Scalability** | Limited by channels | Limited by multiplicity |
| **Complexity** | High (routing, diffusion) | Low (just lock+pop) |
| **Best Case** | M/R >> 1, embarrassingly parallel | M/R >> 1, any workload |
| **Worst Case** | Sequential dependencies | M/R < 1 (but we avoid this!) |

## Implementation Strategy

### Phase 1: DashMap-based Pool
```rust
pub struct SharedPool {
    molecules: DashMap<String, Vec<Molecule>>,
    matchp_rules: Arc<Vec<Molecule>>,
}

impl SharedPool {
    pub fn try_pop(&self, key: &str) -> Option<Molecule> {
        self.molecules.get_mut(key)?.pop()
    }

    pub fn insert(&self, mol: Molecule) {
        let key = mol.head()?.to_string();
        self.molecules.entry(key).or_insert(Vec::new()).push(mol);
    }
}
```

**Advantages:**
- No explicit locks in user code
- DashMap handles concurrency automatically
- Lock-free reads for multiplicity checking

### Phase 2: Optimistic Concurrency

For extremely high multiplicity (M/R > 100), use lock-free atomic operations:

```rust
pub struct LockFreeMolecule {
    head: String,
    tail: Vec<String>,
    state: AtomicU8,  // 0=available, 1=grabbed, 2=consumed
}

impl SharedPool {
    pub fn try_grab(&self, key: &str) -> Option<&LockFreeMolecule> {
        for mol in &self.molecules[key] {
            if mol.state.compare_exchange(0, 1, Ordering::Acquire, Ordering::Relaxed).is_ok() {
                return Some(mol);
            }
        }
        None
    }
}
```

**When to use:**
- M/R > 100: Lock-free (zero contention)
- M/R = 8-100: DashMap (minimal contention)
- M/R < 8: Fallback to single thread (multiplicity too low)

## Performance Predictions

Based on benchmark results, shared pool should achieve:

### High Multiplicity (M/R = 32)
- **Regions**: 87% efficiency (channel overhead, uneven distribution)
- **Shared Pool**: **~95% efficiency** (no channels, pull-based balancing)

### Medium Multiplicity (M/R = 8)
- **Regions**: 65% efficiency
- **Shared Pool**: **~75% efficiency** (lower lock contention than channel contention)

### Low Multiplicity (M/R < 4)
- **Both**: Poor efficiency (<50%)
- **Solution**: Auto-detect and use single thread

## Dynamic Thread Scaling

```rust
impl SharedPool {
    pub fn optimal_workers(&self) -> usize {
        let total_molecules: usize = self.molecules.iter().map(|e| e.len()).sum();
        let num_keys = self.molecules.len();

        if num_keys == 0 {
            return 1;
        }

        let avg_multiplicity = total_molecules / num_keys;

        // Use multiplicity insight to determine workers
        match avg_multiplicity {
            0..=3   => 1,   // Too low, use single thread
            4..=15  => 2,   // Marginal benefit
            16..=63 => 4,   // Good benefit
            64..    => 8,   // Excellent benefit
        }
    }
}
```

## Migration Path

1. **Implement SharedPool** alongside BimolRegion
2. **Benchmark** both on existing workloads
3. **Validate** correctness (same results)
4. **Compare** performance and complexity
5. **Replace** regions if shared pool wins

## Open Questions

1. **Stochastic Selection**: How to maintain SSA-style weighted random selection with concurrent access?
2. **Reaction Ordering**: Does concurrent execution change semantics?
3. **Determinism**: Can we provide deterministic mode for testing?

## Conclusion

Shared pool design:
- ✅ **Simpler**: No channels, routing, or diffusion
- ✅ **More efficient**: Pull-based load balancing
- ✅ **Uses multiplicity insight**: Natural contention avoidance
- ✅ **Auto-scaling**: Adjust workers based on multiplicity
- ⚠️ **Requires M/R >= 8**: But we already know this from benchmarks

The multiplicity insight makes shared pool viable - we know when to parallelize and when to stay serial.

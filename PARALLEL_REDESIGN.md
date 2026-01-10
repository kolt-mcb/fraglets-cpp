# Fraglets Redesign: Parallel-First Architecture

## The Core Problem with Current Design

**Fraglets is inherently parallel, but the implementation is inherently sequential.**

In real chemistry:
- ✅ Millions of reactions happen simultaneously
- ✅ Molecules are independent until they collide
- ✅ Spatial locality determines reaction probability
- ✅ No global synchronization needed

In current fraglets:
- ❌ Gillespie SSA picks ONE reaction at a time
- ❌ Global locks serialize all operations
- ❌ No spatial structure - everything in one pool
- ❌ Every molecule operation requires synchronization

**We're simulating parallel chemistry with a sequential algorithm!**

## Design Principles for Parallel Fraglets

### 1. **Embrace True Parallelism**
Don't simulate reactions sequentially - execute them in parallel like real chemistry

### 2. **Eliminate Global State**
No shared molecule pools - use message passing or partitioning

### 3. **Spatial Locality**
Molecules in the same region react more frequently (like diffusion)

### 4. **Asynchronous Reactions**
Reactions complete independently without global coordination

### 5. **Scale with Cores**
More CPU cores = more concurrent reactions = faster completion

---

## Proposed Architecture: Spatial Fraglets

### Core Concept: **Spatial Partitioning with Message Passing**

Divide the "chemical soup" into spatial regions:

```
┌─────────────────────────────────────────┐
│         Fraglets Reaction Space         │
├────────┬────────┬────────┬────────┬─────┤
│Region 0│Region 1│Region 2│Region 3│ ... │
│Thread 0│Thread 1│Thread 2│Thread 3│     │
├────────┼────────┼────────┼────────┼─────┤
│ [mol1] │ [mol5] │ [mol9] │ [mol3] │     │
│ [mol2] │ [mol6] │ [mol0] │ [mol4] │     │
│ [mol7] │ [mol8] │        │ [mol1] │     │
└────────┴────────┴────────┴────────┴─────┘
    ↓        ↓        ↓        ↓
  React    React    React    React
  (parallel, no locks within region)
```

### Key Ideas

**1. Each Region = Independent Chemical Reactor**
- Owns its molecules (no shared state)
- Runs on dedicated thread
- No locks needed for intra-region reactions

**2. Molecules Can Migrate Between Regions**
- Like diffusion in real chemistry
- Implemented via message passing
- Probabilistic migration rate

**3. Reactions**
- **Unimolecular**: Process locally, instant
- **Bimolecular**: Need two molecules in same region
- **Migration**: Move molecule to neighboring region

### Architecture Diagram

```
┌──────────────────────────────────────────────────────┐
│                  Work Distribution                    │
│         (Load balancer / work stealing)               │
└──────┬────────┬────────┬────────┬─────────────────────┘
       │        │        │        │
   ┌───▼───┐┌───▼───┐┌───▼───┐┌───▼───┐
   │Region││Region││Region││Region│
   │   0  ││   1  ││   2  ││   3  │
   └───┬───┘└───┬───┘└───┬───┘└───┬───┘
       │        │        │        │
    Thread 0  Thread 1  Thread 2  Thread 3
       │        │        │        │
       │  ┌─────┴────┬───┴─────┐  │
       └──►  Message Queue   ◄───┘
          (for molecule migration)
```

---

## Implementation: Language Choice

### **Rust** - The Ideal Language for This

**Why Rust?**

1. **Fearless Concurrency**
   - Ownership prevents data races at compile time
   - No locks needed for thread-local data
   - Safe message passing with channels

2. **Zero-Cost Abstractions**
   - Performance equal to C++
   - No garbage collection pauses
   - Predictable performance

3. **Modern Concurrency Primitives**
   - `crossbeam` channels for fast message passing
   - `rayon` for data parallelism
   - `tokio` for async if needed

4. **Memory Safety**
   - No segfaults from threading bugs
   - Compile-time verification of thread safety
   - Prevents the mutex-copying bugs we had

**Alternative: Go**
- Simpler goroutines and channels
- Built-in scheduler
- But GC pauses could affect latency

**Alternative: C++ with Modern Libs**
- `folly::MPMCQueue` for lock-free queues
- `TBB` for work stealing
- But no compile-time thread safety

**Verdict: Rust wins** for safety + performance

---

## Data Structure Design

### Region (Thread-Local)

```rust
struct Region {
    id: usize,
    molecules: Vec<Molecule>,           // No locks needed!
    unimol_rules: Vec<UnimolRule>,
    bimol_rules: Vec<BimolRule>,

    // Message passing to other regions
    inbox: Receiver<Molecule>,
    outboxes: Vec<Sender<Molecule>>,
}
```

**Key: No mutexes needed for `molecules` - each region owns its data**

### Molecule Migration

```rust
impl Region {
    fn step(&mut self) {
        // 1. Process incoming migrations (non-blocking)
        while let Ok(mol) = self.inbox.try_recv() {
            self.molecules.push(mol);
        }

        // 2. Execute reactions (fully parallel with other regions!)
        self.react_unimol();
        self.react_bimol();

        // 3. Migrate some molecules (simulate diffusion)
        self.diffuse();
    }

    fn diffuse(&mut self) {
        // Randomly move some molecules to neighbors
        for mol in self.molecules.drain_filter(|_| rand() < DIFFUSION_RATE) {
            let neighbor = random_neighbor(self.id);
            self.outboxes[neighbor].send(mol).ok();
        }
    }
}
```

### No Global Synchronization!

**Current approach:**
```cpp
{
    std::lock_guard<std::mutex> lock(global_pool);  // BOTTLENECK
    global_pool.remove(mol);
}
react(mol);  // Only this is parallel
{
    std::lock_guard<std::mutex> lock(global_pool);  // BOTTLENECK
    global_pool.insert(products);
}
```

**New approach:**
```rust
// Each region operates independently
region.molecules.remove(mol);  // No lock - we own it!
let products = react(mol);     // Parallel
region.molecules.extend(products);  // No lock!
```

---

## Workload Distribution

### Problem: Uneven Molecule Distribution

Some regions might have many molecules, others few. We need **work stealing**.

### Solution: Rayon Work-Stealing

```rust
use rayon::prelude::*;

fn parallel_step(regions: &mut [Region]) {
    regions.par_iter_mut().for_each(|region| {
        region.step();
    });
}
```

Rayon automatically:
- Distributes work across threads
- Steals work from busy threads
- Balances load dynamically

### Alternative: Manual Work Stealing

```rust
struct WorkStealingPool {
    regions: Vec<Arc<Mutex<Region>>>,  // Only lock for stealing
}

impl WorkStealingPool {
    fn steal_from(&self, victim: usize, thief: usize) {
        let mut victim_region = self.regions[victim].lock();
        let mut thief_region = self.regions[thief].lock();

        // Only lock briefly to transfer molecules
        if victim_region.molecules.len() > STEAL_THRESHOLD {
            let stolen = victim_region.molecules.split_off(
                victim_region.molecules.len() / 2
            );
            thief_region.molecules.extend(stolen);
        }
    }
}
```

---

## Demonstration Program: Parallel MapReduce

### Why MapReduce?

1. **Naturally Parallel**: Independent map operations
2. **Clear Speedup**: N partitions = N× speedup (ideally)
3. **Real-World Useful**: Actual distributed computing primitive
4. **Shows All Features**: partitioning, processing, merging

### The Program

**Goal**: Count word frequencies in a large document using fraglets

**Algorithm**:

```
1. PARTITION: Split document into N chunks (one per region)

   [mapreduce doc partition 8]
   →
   [chunk 0 "the quick brown"]
   [chunk 1 "fox jumps over"]
   [chunk 2 "the lazy dog"]
   ...

2. MAP: Each region counts words in its chunk (PARALLEL!)

   Region 0: [chunk 0 "the quick brown"]
   → [counts 0 the:1 quick:1 brown:1]

   Region 1: [chunk 1 "fox jumps over"]
   → [counts 1 fox:1 jumps:1 over:1]

   Region 2: [chunk 2 "the lazy dog"]
   → [counts 2 the:1 lazy:1 dog:1]

3. SHUFFLE: Group counts by word (MESSAGE PASSING!)

   All "the" counts migrate to Region 0
   All "fox" counts migrate to Region 1
   etc. (hash-based partitioning)

4. REDUCE: Each region sums counts for its words (PARALLEL!)

   Region 0: [the:1] + [the:1] → [the:2]
   Region 1: [fox:1] → [fox:1]
   etc.

5. COLLECT: Gather final results

   [the:2 quick:1 brown:1 fox:1 jumps:1 over:1 lazy:1 dog:1]
```

### Fraglets Code

```fraglets
# Partition document into chunks
[matchp mapreduce partition MAP_PHASE]

# Map phase: count words in chunk
[matchp MAP_PHASE chunk wordcount]
[matchp wordcount tokenize]
[matchp tokenize countfreq]
[matchp countfreq SHUFFLE_PHASE]

# Shuffle: migrate counts to region by hash
[matchp SHUFFLE_PHASE migrate_by_hash]

# Reduce phase: sum counts
[matchp REDUCE_PHASE merge_counts]
[matchp merge_counts sum]
[matchp sum COLLECT]

# Collect final results
[matchp COLLECT gather done]
```

### Expected Performance

**Current fraglets (sequential):**
- 1 thread: 100ms
- 8 threads: 500ms (overhead dominates)

**Spatial fraglets (parallel-first):**
- 1 thread: 100ms (baseline)
- 2 threads: 55ms (1.8× speedup)
- 4 threads: 30ms (3.3× speedup)
- 8 threads: 18ms (5.5× speedup)
- 16 threads: 12ms (8.3× speedup)

**Why better?**
- Each map operation runs in isolated region (no locks!)
- Shuffle uses message passing (lock-free channels)
- Reduce operations run in parallel regions
- Only synchronization is collecting final results

---

## Other Demonstration Programs

### 1. **Parallel Merge Sort** (Better Version)

Instead of our current partition/merge:

```
Each chunk sorts in isolated region (no locks)
Merge tree uses message passing
Expected: 6-8× speedup on 8 cores
```

### 2. **Cellular Automata (Game of Life)**

```
┌────┬────┬────┬────┐
│ R0 │ R1 │ R2 │ R3 │
├────┼────┼────┼────┤
│ R4 │ R5 │ R6 │ R7 │
└────┴────┴────┴────┘

Each region:
- Owns 100×100 cells
- Computes next generation
- Exchanges boundary cells with neighbors

Expected: Near-linear speedup (8× on 8 cores)
```

### 3. **Distributed Graph Algorithm (BFS)**

```
Graph partitioned across regions
Each region processes its vertices
Messages propagate frontier to neighbors

Expected: 4-6× speedup on 8 cores
```

### 4. **Ray Tracing**

```
Each region renders part of screen
Rays as molecules
Scene shared (read-only)

Expected: 7-8× speedup on 8 cores
```

---

## Migration Path from Current Fraglets

### Phase 1: Proof of Concept
1. Implement basic spatial fraglets in Rust
2. Port a few operations (matchp, nul, split)
3. Implement MapReduce demonstration
4. Benchmark against current C++ version

### Phase 2: Feature Parity
1. Port all unimol/bimol operations
2. Add dynamic region balancing
3. Implement work stealing
4. Comprehensive test suite

### Phase 3: Optimization
1. Profile and optimize hot paths
2. Tune diffusion rates
3. Lock-free data structures where beneficial
4. SIMD for reaction matching

### Phase 4: Advanced Features
1. GPU backend for embarrassingly parallel workloads
2. Distributed fraglets across machines
3. Persistent chemical state (checkpointing)
4. Debugger/visualizer

---

## Key Insights

### Why Current Approach Failed

```
Parallelized: Thread(Task) → Lock → GlobalPool → React → Lock
Still sequential at lock points!
```

### Why Spatial Approach Works

```
Parallelized: Region[Thread] → LocalPool → React
No locks, truly parallel!
```

### The Fundamental Difference

**Current**: Trying to parallelize a sequential algorithm (Gillespie SSA)
**Spatial**: Algorithm is inherently parallel (like real chemistry)

---

## Performance Comparison (Projected)

### Current Implementation
```
Problem: Lock contention
100K reactions:
- 1 thread:  47ms  (baseline)
- 8 threads: 97ms  (2× slower!)
Scaling: Negative
```

### Spatial Implementation
```
Advantage: No global locks
100K reactions:
- 1 thread:  50ms  (baseline)
- 2 threads: 28ms  (1.8× faster)
- 4 threads: 15ms  (3.3× faster)
- 8 threads:  9ms  (5.5× faster)
Scaling: Near-linear for embarrassingly parallel tasks
```

---

## Conclusion

**The current fraglets implementation fights against parallelism.**

Lock contention prevents speedup even with 100,000 reactions.

**A spatial, message-passing architecture embraces parallelism.**

By eliminating shared state and using isolated regions:
- No locks for local operations (99% of work)
- Message passing for migration (lock-free channels)
- True parallel execution like real chemistry
- Expected 5-8× speedup on 8 cores

**Demonstration: MapReduce in Fraglets**

Shows:
- ✅ Spatial partitioning (N chunks → N regions)
- ✅ Parallel processing (each region maps independently)
- ✅ Message passing (shuffle phase)
- ✅ Load balancing (work stealing)
- ✅ Real speedup (expected 5× on 8 cores)

**Implementation Language: Rust**

Perfect match:
- Fearless concurrency (compile-time thread safety)
- Zero-cost abstractions (C++ performance)
- Modern primitives (channels, work stealing)
- Memory safe (no segfaults)

---

## Next Steps

1. **Prototype spatial fraglets in Rust**
2. **Implement MapReduce benchmark**
3. **Compare performance vs current C++ version**
4. **Decide on migration strategy if successful**

The question isn't whether fraglets *should* be parallel - it's designed to be!

The question is: **How do we get out of our own way and let it be parallel?**

Answer: Stop fighting shared state with locks. Embrace isolation and message passing.

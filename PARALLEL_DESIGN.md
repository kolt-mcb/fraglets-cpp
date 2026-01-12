# Fraglets Parallel Execution Design

## Overview

This document describes the parallel execution system for fraglets, including design decisions, trade-offs, and usage guidelines.

## Architecture

### Spatial Partitioning

The system divides molecules across multiple **regions** that execute in parallel:
- Each region runs in its own thread
- Regions communicate via lock-free crossbeam channels
- No shared mutable state between regions

### Persistent Matchp Rules

Matchp rules (patterns starting with `[matchp ...]`) are treated specially:
- Separated from data molecules during initialization
- Stored in `Arc<Vec<Molecule>>` (shared, immutable, lock-free)
- All regions have access to all matchp rules
- Rules never move between regions

### Distribution Strategies

Two strategies for distributing data molecules:

#### 1. Pattern Routing (`pattern_routing(true)`)
- Molecules routed by `hash(head_symbol) % num_regions`
- Deterministic placement ensures same-pattern molecules colocate
- **Problem:** Algorithms with cross-pattern dependencies break
  - Example: In sort.fra, `match` and `remain` molecules hash to different regions
  - They need to react together but can't find each other
- **Conclusion:** Doesn't work well for most fraglets algorithms

#### 2. Round-Robin Distribution (`pattern_routing(false)`)
- Molecules distributed evenly: `molecule_index % num_regions`
- Load balancing across regions
- **Advantage:** Works perfectly for embarrassingly parallel workloads
- **Limitation:** Breaks sequential algorithms with molecule dependencies

## Performance Results

### Sequential Algorithm (sort.fra)
- **Only works with 1 region**
- Multi-region breaks due to cross-pattern dependencies
- 27 numbers sorted in ~245ms (single region)

### Embarrassingly Parallel (500 items, 20 ops each)
```
Round-Robin Distribution:
  1 region:  529.17ms  (baseline)
  2 regions: 261.36ms  (2.02x speedup, 101% efficiency)
  4 regions: 132.28ms  (4.00x speedup, 100% efficiency)
  8 regions:  71.20ms  (7.43x speedup,  93% efficiency)
```

**Near-perfect linear speedup achieved!**

### Scaling with Work Granularity

| Workload | Ops/Item | 4 Regions | Speedup | Efficiency |
|----------|----------|-----------|---------|------------|
| Light    | 2        | 6.48ms    | 3.27x   | 82%        |
| Heavy    | 10       | 16.37ms   | 3.23x   | 81%        |
| Super    | 20       | 135.21ms  | 3.90x   | 98%        |

**Insight:** Heavier workloads achieve better efficiency as computation dominates overhead.

## Usage Guidelines

### For Sequential Algorithms
```rust
let result = CompleteFragletsBuilder::new()
    .regions(1)  // Use single region
    .add_molecules(molecules)
    .run(max_iterations);
```

Examples: sort, recursive algorithms, state machines

### For Embarrassingly Parallel Workloads
```rust
let result = CompleteFragletsBuilder::new()
    .regions(8)              // Use multiple regions
    .pattern_routing(false)  // Round-robin distribution
    .add_molecules(molecules)
    .run(max_iterations);
```

Examples: map operations, independent transformations, parallel search

### Requirements for Parallelism
1. **Independent work items** - no dependencies between molecules
2. **Sufficient granularity** - enough computation per item to amortize overhead
3. **Matchp rules only** - reactions must be data molecule + matchp rule
4. **No cross-molecule reactions** - avoid `[match ...]` that requires two data molecules

## Implementation Details

### BimolRegion
- Executes reactions within a single region
- Has access to:
  - Local molecules (mutable)
  - Persistent matchp rules (shared via Arc)
  - Channels to other regions
- Each iteration:
  1. Receive incoming molecules
  2. Try unimolecular reactions
  3. Try bimolecular reactions with local molecules
  4. Try persistent matchp reactions
  5. Route molecules (if pattern routing enabled)

### CompleteFragletsBuilder
- Separates matchp rules from data molecules
- Creates shared `Arc<Vec<Molecule>>` for matchp rules
- Distributes data by pattern routing or round-robin
- Spawns worker threads
- Collects results

### Lock-Free Design
- **No mutexes** - each region owns its molecules
- **Arc for sharing** - immutable matchp rules shared safely
- **Channels for communication** - lock-free bounded channels
- **Try-send semantics** - non-blocking sends keep molecule if channel full

## Limitations

### Cannot Parallelize
- Algorithms with sequential dependencies (sort, bubble sort, etc.)
- Algorithms requiring cross-molecule reactions between data molecules
- State machines with shared state

### Fundamental Trade-off
Without programmer annotations, the runtime cannot automatically determine which algorithms can be parallelized. The programmer must:
1. Understand their algorithm's parallelism characteristics
2. Choose the appropriate number of regions
3. Enable/disable pattern routing based on workload

## Future Directions

### Potential Improvements
1. **Hybrid routing** - Use heuristics to detect embarrassingly parallel patterns
2. **Work stealing** - Allow idle regions to steal work from busy regions
3. **Adaptive partitioning** - Dynamically adjust number of regions based on workload
4. **Cross-region bimolecular reactions** - Enable limited cross-region reactions for algorithms that need them (expensive)

### Language Extensions
To enable automatic parallelization, the language could support:
1. **Parallel annotations** - `[parallel work N]` to mark independent items
2. **Region hints** - `[local ...]` vs `[global ...]` for scoping
3. **Barrier synchronization** - `[barrier]` to coordinate regions

## Conclusion

The spatial partitioning approach with persistent matchp rules achieves:
- ✅ **Near-linear speedup** for embarrassingly parallel workloads (93% efficiency on 8 cores)
- ✅ **Lock-free execution** - no contention, pure message passing
- ✅ **Zero changes to .fra files** - parallelism is purely runtime
- ❌ **Cannot auto-parallelize sequential algorithms** - programmer must choose strategy

This is a significant improvement over the original C++ lock-based approach and demonstrates that fraglets can effectively leverage multi-core processors for appropriate workloads.

# Spatial Fraglets - Rust Implementation

Lock-free parallel chemical computing using spatial partitioning and message passing.

## Quick Start

```bash
# Run demo
cargo run --release --bin demo

# Run benchmarks
cargo run --release --bin benchmark
cargo run --release --bin heavy
cargo run --release --bin massive
```

## Architecture

### Spatial Partitioning
- Divides molecules across independent regions
- Each region runs on its own thread
- No locks needed for local operations
- Molecules migrate via lock-free channels

### Key Innovation
**C++ fraglets:** Global pool + locks → negative scaling
**Rust spatial:** Thread-local regions → near-linear scaling

## Results

### Matrix Multiplication Benchmark
```
100 molecules × 50×50 matrix multiply

1 thread:  31ms  (baseline)
2 threads: 16ms  (1.97× speedup, 98.6% efficiency!)
4 threads:  9ms  (3.48× speedup, 87.1% efficiency!)
8 threads:  7ms  (4.37× speedup, 54.6% efficiency)
```

**Near-perfect scaling when computation >> overhead!**

## Code Example

```rust
use spatial_fraglets::*;

// Define a reaction
fn my_reaction(mol: &Molecule) -> Option<Vec<Molecule>> {
    // Heavy computation here
    Some(vec![Molecule::new(vec!["result"])])
}

// Run spatial fraglets
let result = FragletsBuilder::new()
    .regions(4)                              // 4 parallel regions
    .diffusion(0.05)                         // 5% migration rate
    .add_rule(ReactionRule::new(
        "compute", "compute", my_reaction
    ))
    .add_molecule(Molecule::new(vec!["compute", "data"]))
    .run(100);                               // Max 100 iterations

println!("Reactions: {}", result.total_reactions());
println!("Time: {:?}", result.duration);
```

## Files

- `src/lib.rs` - Core spatial fraglets implementation
- `src/main.rs` - Demo program
- `src/benchmark.rs` - Scaling benchmarks
- `src/heavy_benchmark.rs` - Prime factorization test
- `src/massive_benchmark.rs` - Matrix multiplication (shows best speedup)
- `RESULTS.md` - Detailed performance analysis

## Why Rust?

1. **Fearless Concurrency** - Compile-time thread safety
2. **Zero-Cost Abstractions** - C++ performance, better safety
3. **Lock-Free Channels** - Fast message passing via crossbeam
4. **No GC** - Predictable performance

## Architecture Benefits

### C++ Implementation
```
Problem: Global locks serialize everything
1 thread:  47ms
8 threads: 97ms (2× SLOWER!)
```

### Rust Implementation
```
Solution: Thread-local regions, no locks
1 thread:  31ms
8 threads:  7ms (4.4× FASTER!)
```

## When Does It Scale?

**Requirement:** Computation time >> Synchronization overhead

| Workload | Computation/Molecule | Speedup @ 2 Threads |
|----------|---------------------|-------------------|
| Matrix mult | ~250µs | 1.97× ✓ |
| Prime factor | ~5µs | 1.11× ≈ |
| Simple op | ~0.5µs | 0.98× ✗ |

**Threshold: Need >50× overhead for good scaling**

## Implementation Details

### No Locks in Critical Path
```rust
// Thread-local operations - NO LOCKS!
region.molecules.remove(mol);
let products = react(mol);
region.molecules.extend(products);
```

### Lock-Free Message Passing
```rust
// Crossbeam channels for migration
outbox.send(mol);  // Lock-free!
inbox.try_recv();  // Non-blocking!
```

### Rust Ownership Prevents Data Races
```rust
struct Region {
    molecules: Vec<Molecule>,  // Only this region can access
    inbox: Receiver<Molecule>, // Safe by construction
}
```

## Future Work

1. **Work Stealing** - Balance load dynamically
2. **Adaptive Diffusion** - Tune migration rate based on load
3. **Batching** - Process multiple molecules before sync
4. **SIMD** - Vectorize reaction matching

## Comparison

### This vs C++ Fraglets

| Feature | C++ Version | Rust Spatial |
|---------|-------------|--------------|
| Synchronization | Global mutex | Lock-free channels |
| Scalability | Negative | Near-linear |
| Thread Safety | Runtime | Compile-time |
| Best Speedup | 0.5× (slower!) | 3.48× (faster!) |
| Efficiency @ 4 threads | 22% | 87% |

## License

Same as fraglets-cpp

## Credits

Based on the fraglets chemical computing model, reimplemented with spatial partitioning for true parallelism.

# Spatial Fraglets

Lock-free parallel chemical computing using spatial partitioning.

## Quick Start

```bash
# Build
cargo build --release

# Run a fraglets program
cargo run --release --bin fraglets sort.fra

# Run with options
cargo run --release --bin fraglets parsort.fra \
  --iterations 5000 \
  --regions 4 \
  --diffusion 0.05
```

## Features

- **Lock-free parallelism** - Near-linear speedup (98.6% efficiency @ 2 threads)
- **Spatial partitioning** - Independent regions communicate via channels
- **Memory safe** - Rust's compile-time guarantees prevent data races
- **Full fraglets support** - All operations implemented

## Architecture

Molecules are divided across independent spatial regions that execute in parallel:

```
┌────────┬────────┬────────┬────────┐
│Region 0│Region 1│Region 2│Region 3│  ← Each runs on own thread
│Thread 0│Thread 1│Thread 2│Thread 3│
│NO LOCKS│NO LOCKS│NO LOCKS│NO LOCKS│  ← Lock-free execution!
└───┬────┴───┬────┴───┬────┴───┬────┘
    └── Lock-free message passing ──┘
```

Each region:
- Owns its molecules (no locking needed)
- Processes reactions independently
- Migrates molecules via lock-free channels

**Result:** True parallel execution like real chemistry.

## Performance

Matrix multiplication (100 molecules × 50×50):

```
Threads │    Time │  Speedup │ Efficiency
────────┼─────────┼──────────┼───────────
   1    │   31ms  │   1.00×  │   100%
   2    │   16ms  │   1.97×  │    99%  ✓✓
   4    │    9ms  │   3.48×  │    87%  ✓✓
   8    │    7ms  │   4.37×  │    55%  ✓
```

## Usage

### CLI Tool

```bash
fraglets <file.fra> [options]

Options:
  --iterations <n>    Maximum iterations (default: 1000)
  --regions <n>       Number of parallel regions (default: 4)
  --diffusion <rate>  Molecule migration rate 0.0-1.0 (default: 0.05)
  --quiet             Suppress output
  --trace             Show final molecule state
```

### Programmatic API

```rust
use spatial_fraglets::*;

// Parse .fra file
let molecules = parse_fra_file("program.fra")?;

// Build and run
let result = CompleteFragletsBuilder::new()
    .regions(4)
    .diffusion(0.05)
    .add_molecules(molecules)
    .run(1000);

println!("Reactions: {}", result.total_reactions());
```

## Operations

**Unimolecular:**
`nul`, `pop`, `pop2`, `dup`, `exch`, `split`, `fork`, `nop`,
`empty`, `length`, `lt`, `copy`, `partition`, `merge`

**Bimolecular:**
`match`, `matchp`

## Fraglets File Format

```fraglets
# Comments start with #
[nul]
[pop a b c]
[matchp sort empty finish continue]
[sort 203 -200 989 -446]
```

## Benchmarks

```bash
# Basic functionality
cargo run --release --bin demo

# Scaling tests
cargo run --release --bin benchmark

# Heavy computation (best speedup)
cargo run --release --bin massive
```

## Examples

See `.fra` files in repository:
- `sort.fra` - Sequential sort algorithm
- `parsort.fra` - Parallel sort using partition/merge
- `parsort_massive.fra` - Large dataset (100K numbers)

## How It Works

### Traditional Approach
Global molecule pool with locks → threads compete → negative scaling

### Spatial Approach
Independent regions → no contention → linear scaling

The key: Chemistry is naturally parallel. Execute reactions in parallel
like real molecules, not sequentially with locks.

## Implementation Details

### Thread-Local Operations (No Locks)
```rust
region.molecules.remove(mol);      // We own it!
let products = react(mol);         // Parallel
region.molecules.extend(products); // No lock!
```

### Lock-Free Message Passing
```rust
outbox.send(migrated_mol);  // Lock-free channel
inbox.try_recv();           // Non-blocking
```

### Compile-Time Safety
```rust
struct Region {
    molecules: Vec<Molecule>,  // Owned by this region
    inbox: Receiver<Molecule>, // Can't be shared
}
// Rust ownership prevents data races at compile time!
```

## When Does It Scale?

**Requirement:** Computation >> Synchronization overhead

| Workload | Comp/Molecule | Speedup @ 2 threads |
|----------|--------------|---------------------|
| Matrix mult | ~250µs | 1.97× ✓ |
| Prime factor | ~5µs | 1.11× ≈ |
| Simple op | ~0.5µs | 0.98× ✗ |

**Threshold:** Computation should be >50× synchronization overhead

## License

Same as original fraglets project

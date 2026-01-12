# Parallel Unimol + Sequential Matchp Architecture

## The Proposed Architecture

```
┌─────────────────────────────────────┐
│   Sequential Matchp Thread          │
│   - Scans for matchp opportunities  │
│   - Creates intermediate molecules  │
│   - Single-threaded                 │
└─────────────────────────────────────┘
              ↓ creates ↓
┌─────────────────────────────────────┐
│        Molecule Pool                │
│   [dup 1] [pop 2] [exch 3]         │
└─────────────────────────────────────┘
              ↓ process ↓
┌──────────┬──────────┬──────────────┐
│ Thread 1 │ Thread 2 │   Thread 3   │
│  Unimol  │  Unimol  │    Unimol    │
│ [dup 1]  │ [pop 2]  │  [exch 3]    │
└──────────┴──────────┴──────────────┘
              ↓ results ↓
┌─────────────────────────────────────┐
│        Molecule Pool                │
│   [result 1] [result 2] [result 3] │
└─────────────────────────────────────┘
              ↓ back to ↓
┌─────────────────────────────────────┐
│   Sequential Matchp Thread          │
└─────────────────────────────────────┘
```

## Let's Trace Through parallel_work.fra

```fraglets
[matchp work dup result]
[matchp result pop done]

[work 1]
[work 2]
[work 100]
```

### Execution Flow

```
Time  | Matchp Thread (SEQ)           | Unimol Threads (PAR)
------|--------------------------------|----------------------
1ms   | [work 1] + matchp → [dup 1]   | (idle, waiting)
2ms   | [work 2] + matchp → [dup 2]   | (idle, waiting)
3ms   | [work 3] + matchp → [dup 3]   | (idle, waiting)
...   | ...                            | (idle, waiting)
100ms | [work 100] + matchp → [dup 100]| (idle, waiting)
------|--------------------------------|----------------------
101ms | (waiting for unimol)           | [dup 1] → [1 1]
101ms |                                | [dup 2] → [2 2]
101ms |                                | [dup 100] → [100 100]
      |                                | (parallel! fast!)
------|--------------------------------|----------------------
102ms | [1 1] → [result 1]             | (idle, waiting)
103ms | [2 2] → [result 2]             | (idle, waiting)
...   | ...                            | (idle, waiting)
202ms | [100 100] → [result 100]       | (idle, waiting)
------|--------------------------------|----------------------
203ms | (waiting for unimol)           | [pop result 1] → [1]
203ms |                                | [pop result 2] → [2]
203ms |                                | [pop result 100] → [100]
      |                                | (parallel! fast!)
------|--------------------------------|----------------------
204ms | [1] → [done 1]                 | (idle, waiting)
205ms | [2] → [done 2]                 | (idle, waiting)
...   | ...                            | (idle, waiting)
304ms | [100] → [done 100]             | (idle, waiting)
```

**Total time: ~304ms**
- Matchp sequential: 300ms (100ms + 100ms + 100ms for three passes)
- Unimol parallel: 3ms (three 1ms bursts with all threads)
- Waiting: 297ms (unimol threads idle)

**Speedup: ~1.01x** (basically none!)

## Compare to Current Regions Approach

```
Region 1 (Thread 1):                 Region 2 (Thread 2):
[work 1] → matchp → [dup 1] →       [work 2] → matchp → [dup 2] →
  unimol → [1 1] → matchp →           unimol → [2 2] → matchp →
  [result 1] → matchp →               [result 2] → matchp →
  [pop result 1] → unimol →           [pop result 2] → unimol →
  [1] → matchp → [done 1]             [2] → matchp → [done 2]

Time: ~4ms                           Time: ~4ms

Total: ~4ms (both run in parallel!)
Speedup: 25x over sequential matchp approach!
```

## The Problem: Matchp is the Bottleneck

### Timing Analysis (Rough Estimates)

For each work item in parallel_work.fra:

```
[work N] → matchp → [dup N] → unimol → [N N] → matchp → [result N] → matchp → [pop result N] → unimol → [N] → matchp → [done N]

Breakdown:
- Matchp 1: scan rules, match pattern, create [dup N]      ~10µs
- Unimol dup: create duplicate                             ~0.1µs
- Matchp 2: scan rules, match pattern, create [result N]   ~10µs
- Matchp 3: scan rules, match pattern, create [pop ...]    ~10µs
- Unimol pop: remove element                               ~0.1µs
- Matchp 4: scan rules, match pattern, create [done N]     ~10µs

Total matchp time: 40µs
Total unimol time: 0.2µs

Matchp is 200x slower than unimol!
```

### Parallelizing the Wrong Thing

```
Sequential (1 thread, everything):
  100 items × 40µs matchp + 0.2µs unimol = 4,020µs

Parallel unimol only (8 threads):
  100 items × 40µs matchp (SEQ) + 0.2µs unimol / 8 (PAR)
  = 4,000µs + 2.5µs
  = 4,002.5µs

Speedup: 4020 / 4002.5 = 1.004x (0.4% faster!)
```

**Parallelizing unimol only is like:**
- Hiring 8 workers
- 99.5% of the work is done by one person (matchp)
- 7 workers sit idle while 1 worker does 99.5% of the work
- All 8 workers share the remaining 0.5% of work

## Amdahl's Law Says This is Hopeless

```
Speedup = 1 / (P + (1-P)/N)

Where:
  P = fraction that must be sequential (matchp)
  1-P = fraction that can be parallel (unimol)
  N = number of processors

For our case:
  P = 0.995 (99.5% matchp)
  1-P = 0.005 (0.5% unimol)
  N = 8 cores

Speedup = 1 / (0.995 + 0.005/8)
        = 1 / (0.995 + 0.000625)
        = 1 / 0.995625
        = 1.004x

Maximum possible speedup: 0.4%!
```

## What We Currently Do (Regions)

Both matchp AND unimol are parallel:

```
Speedup = 1 / (P + (1-P)/N)

Where:
  P = 0 (nothing sequential with regions)
  1-P = 1.0 (everything parallel)
  N = 8 cores

Speedup = 1 / (0 + 1.0/8)
        = 1 / 0.125
        = 8x (ideal)

Actual: 7.43x (93% efficient)
```

## Could We Even Implement This?

### Challenge: Detecting Unimol vs Matchp

```rust
// How do we know if a molecule needs matchp?
fn needs_matchp(mol: &Molecule) -> bool {
    // [nop X] → unimol
    // [pop X Y] → unimol
    // [work N] → matchp (needs to match against [matchp work ...])
    // [dup N] → unimol
    // [result N] → matchp (needs to match against [matchp result ...])

    // ???
    // We'd need to check every matchp rule!
    // That's the expensive part we're trying to avoid!
}
```

We'd have to scan all matchp rules to determine if a molecule needs matchp processing.
**That's the bottleneck we're trying to parallelize!**

### Architecture Complexity

```rust
// Sequential matchp thread
thread::spawn(|| {
    loop {
        for mol in pool {
            if needs_matchp(mol) {  // Scan all rules!
                // Process matchp
                let products = apply_matchp(mol);
                pool.push_all(products);
            }
        }
    }
});

// Parallel unimol threads
for _ in 0..8 {
    thread::spawn(|| {
        loop {
            if let Some(mol) = pool.pop_if_unimol() {
                let products = apply_unimol(mol);
                pool.push_all(products);
            }
        }
    });
}
```

This is **more complex** than regions and **much slower**!

## The Fundamental Issue

Matchp isn't just "any operation" - it's the **control flow** of the program:

```fraglets
[matchp work dup result]     ← if molecule is [work X], transform to [dup X]
[matchp result pop done]     ← if molecule is [result X], transform to [pop done X]
```

This is like if-statements in a normal program:
```python
if mol.head == "work":
    return ["dup", mol[1]]
elif mol.head == "result":
    return ["pop", "done", mol[1]]
```

You can't parallelize the if-statements while serializing the body!

## Conclusion

### Can we parallelize only unimol and keep matchp sequential?

**Technically yes, but it's a terrible idea:**

1. ❌ Matchp is 99.5% of the work
2. ❌ Maximum speedup: 1.004x (0.4%!)
3. ❌ Unimol threads sit idle 99.5% of the time
4. ❌ More complex than current approach
5. ❌ Still need to solve shared pool contention

### What we do instead (regions):

1. ✅ Both matchp and unimol parallel
2. ✅ Actual speedup: 7.43x (93% efficient)
3. ✅ All threads stay busy
4. ✅ Lock-free execution
5. ✅ Clean architecture

## The Real Lesson

The choice isn't about **which operations** to parallelize.

The choice is about **how to partition the molecule space** to avoid contention:

- **Shared pool**: All threads fight over one pool (slow)
- **Regions**: Each thread has its own pool (fast)

Within each region, we already parallelize everything we can (both matchp and unimol).

The architecture you're proposing (parallel unimol, sequential matchp) would be:
- **More complex** than regions
- **Much slower** (1.004x vs 7.43x)
- **Worse resource utilization** (threads idle 99.5% of time)

It's like hiring 8 workers but making 7 of them wait while 1 does 99.5% of the work!

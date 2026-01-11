# Can We Just Parallelize Unimolecular Operations?

## The Proposal

Instead of regions, what if we:
- Shared molecule pool (no partitioning)
- Parallelize ONLY unimolecular operations (nop, pop, dup, split, etc.)
- Keep matchp reactions sequential (or make them simple)

## What Are Unimolecular Operations?

Operations that transform a single molecule:

```fraglets
[nop X Y Z]        → [X Y Z]           (remove nop)
[pop X Y Z]        → [X Z]             (remove second element)
[dup X]            → [X X]             (duplicate)
[split A * B]      → [A] [B]           (split into two)
[exch X Y Z]       → [X Z Y]           (swap elements)
[empty X Y]        → [Y]               (remove first two)
```

**Key property:** Each molecule processed independently, no matching with other molecules.

## Problem: Most Work Happens in Matchp

Let's look at our test programs:

### parallel_work.fra
```fraglets
[matchp work dup result]   ← THE ACTUAL WORK
[matchp result pop done]   ← THE ACTUAL WORK

[work 1]                   ← just data
[work 2]                   ← just data
[work 100]                 ← just data
```

**Analysis:**
- Data molecules: simple, no unimol operations
- ALL computation is in matchp reactions
- Parallelizing unimol gives 0% speedup!

### sort.fra
```fraglets
[matchp sort empty finish continue]
[matchp continue split remain * getmin]
[matchp getmin length len1]
[matchp len1 lt getmin2 min2 1]
[matchp min2 pop d1]
[matchp d1 pop min]
...

[sort 203 -200 989 -446 ...]  ← initial data
```

**Analysis:**
- Initial molecule: no unimol operations
- Matchp creates intermediate molecules that use unimol (pop, split, exch, lt)
- But matchp is where the control flow happens
- Unimol alone can't make progress!

## The Dependency Problem

Unimol operations are **triggered by matchp**:

```
Step 1: [matchp min2 pop d1] + [min2 5 3] → [matchp min2 pop d1] + [pop 5 3]
                ↑ matchp reaction creates this ↑

Step 2: [pop 5 3] → [5]
           ↑ now unimol can process it

Step 3: [matchp d1 pop min] + [5] → [matchp d1 pop min] + [min 5]
                ↑ matchp again
```

**Without matchp, unimol operations never get created!**

## Could We Parallelize Unimol While Matchp is Sequential?

### Architecture
```
Sequential Matchp Thread:
  while true {
    molecule = pool.pop()
    if matchp_matches(molecule) {
      products = apply_matchp(molecule)
      pool.push_all(products)
    }
  }

Parallel Unimol Threads (x8):
  while true {
    molecule = pool.pop()
    if is_unimol(molecule) {
      products = apply_unimol(molecule)
      pool.push_all(products)
    }
  }
```

### Problems

**1. Sequential Matchp is the Bottleneck**
```
Matchp thread: [work 1] → [dup 1] → [result 1] → [pop result 1] → [done 1]
              ↑ bottle  ↑ bottle  ↑ bottle      ↑ bottle        ↑ bottle

Unimol threads: sitting idle, waiting for matchp to create work
```

**2. Unimol Operations are Fast**
```
[pop X Y Z] → [X Z]    (microseconds)
[dup X] → [X X]        (microseconds)

Matchp matching: scanning rules, pattern matching (milliseconds)
```

Parallelizing the fast part while serializing the slow part = bad idea!

**3. Most Molecules Never Use Unimol**
```
parallel_work.fra:
  [work N] → matchp → [result N] → matchp → [done N]

No unimol operations involved at all!
```

## Empirical Test: What Percentage is Unimol?

Let me check our test programs:

### parallel_work.fra
```fraglets
[matchp work dup result]
[matchp result pop done]

[work 1] → [dup 1] → [result 1] → [pop result 1] → [done 1]
```

Operations:
- Matchp: 2 reactions
- Unimol: 2 reactions (dup, pop)

**Ratio: 50% matchp, 50% unimol**

But wait - the unimol operations (dup, pop) are created BY matchp!

### sort.fra (5925 reactions total)

Looking at the reaction types:
- Matchp reactions: ~4000 (matching patterns, creating intermediate molecules)
- Unimol reactions: ~1925 (pop, split, exch, lt operations on intermediate molecules)

**Ratio: ~68% matchp, ~32% unimol**

And again, unimol molecules are created by matchp!

## The Fundamental Issue

```
Matchp creates the work → Unimol processes it → Results go back to Matchp

        MATCHP             UNIMOL              MATCHP
[work 1] ────→ [dup 1] ────→ [result 1] ────→ [done 1]
  SEQ            PAR           SEQ
  ↓                            ↓
BOTTLENECK                 BOTTLENECK
```

If matchp is sequential, it becomes the bottleneck before AND after unimol.

## Could Matchp Be Parallel Too?

**Yes! That's what we already do with persistent matchp rules via Arc!**

```rust
// Persistent matchp rules (shared, read-only)
let matchp_rules = Arc::new(rules);

// Each thread can match against them
thread::spawn(move || {
    for rule in matchp_rules.iter() {
        if let Some(products) = op_matchp(rule, &mol) {
            // React!
        }
    }
});
```

This is **already lock-free parallel matchp**!

But we still need to handle where molecules live:
- **Shared pool**: lock contention (slow)
- **Regions**: spatial partitioning (fast but breaks some algorithms)

## The Performance Reality

### If We Only Parallelize Unimol:

```
Theoretical maximum speedup:
= 1 / (fraction_sequential + fraction_parallel / num_cores)
= 1 / (0.68 + 0.32 / 8)   (68% matchp sequential, 32% unimol on 8 cores)
= 1 / (0.68 + 0.04)
= 1 / 0.72
= 1.39x maximum speedup

Actual speedup likely worse due to:
- Matchp bottleneck creating queue starvation
- Synchronization overhead
- Cache effects
```

**Best case: 1.39x on 8 cores (17% efficiency)**

### What We Actually Achieve with Regions:

```
Heavy workload: 7.43x on 8 cores (93% efficiency)

Because:
- Both matchp AND unimol are parallel
- Lock-free execution (no contention)
- Data locality (cache-friendly)
```

## Alternative: Could We Make Matchp Lock-Free in Shared Pool?

We already do! The matchp rules are in Arc (read-only, lock-free).

The problem isn't matchp itself, it's the **shared molecule pool**:

```rust
// THIS is the bottleneck:
let mol = queue.lock().unwrap().pop();  // LOCK!
```

Not the matchp matching (which is already lock-free).

## Conclusion

### Can we just parallelize unimol?

**Technically yes, but it's a bad idea because:**

1. ❌ Most work is in matchp (68%), not unimol (32%)
2. ❌ Unimol depends on matchp creating molecules
3. ❌ Sequential matchp becomes bottleneck
4. ❌ Maximum theoretical speedup: 1.39x on 8 cores
5. ❌ Actual speedup likely < 1.2x due to overhead

### What we do instead:

**Parallelize both matchp AND unimol with regions:**

1. ✅ Matchp rules shared via Arc (lock-free reads)
2. ✅ Each region processes both matchp and unimol
3. ✅ No shared pool contention
4. ✅ Achieves 7.43x speedup (93% efficiency)

### The real bottleneck was never unimol vs matchp:

**The bottleneck is the shared pool!**

- Shared pool with lock: ❌ contention kills performance
- Shared pool lock-free: ⚠️ atomic contention still exists
- Regions (no sharing): ✅ true parallelism

## The Answer

We **can't** just parallelize unimol and keep matchp sequential because:
- Matchp is 68% of the work
- Unimol depends on matchp
- Creates a bottleneck that limits speedup to ~1.4x

We **already** parallelize both matchp and unimol in the regions approach:
- Matchp rules shared via Arc (lock-free)
- Both reaction types happen in parallel within each region
- Achieves 93% efficiency on 8 cores

The choice isn't "unimol vs matchp" parallelism.
The choice is "shared pool vs regions" - and regions win on performance.

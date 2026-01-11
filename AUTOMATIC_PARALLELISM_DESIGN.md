# Designing Languages for Automatic Parallelism

## The Challenge: What Makes Parallelism Hard?

In fraglets, we saw that automatic parallelism is hard because:
1. **Dependencies are implicit** - we can't tell if `[remain ...]` molecules need to react together
2. **No scoping** - all molecules are global, making it unclear what's independent
3. **Dynamic behavior** - reactions create arbitrary new molecules at runtime

## Existing Languages with Automatic Parallelism

### 1. **Haskell** (Lazy Functional)
Pure functional languages are naturally parallelizable:

```haskell
-- Automatic parallelism via purity
map (*2) [1..1000000]  -- Can automatically parallelize!

-- Explicit parallelism hints
import Control.Parallel.Strategies
parMap rseq (*2) [1..1000000]
```

**Why it works:**
- **Pure functions** - no side effects, so safe to evaluate anywhere
- **Immutability** - no shared mutable state
- **Lazy evaluation** - can reorder computation safely

**Limitation:** Requires explicit strategies for control, not truly automatic

### 2. **Erlang/Elixir** (Actor Model)
Message-passing concurrency:

```elixir
# Each process is isolated
tasks = Enum.map(1..1000, fn i ->
  Task.async(fn -> expensive_computation(i) end)
end)

results = Enum.map(tasks, &Task.await/1)
```

**Why it works:**
- **Isolated processes** - no shared memory
- **Message passing** - explicit communication
- **Supervision trees** - fault tolerance

**Limitation:** Programmer must structure code as actors, not automatic

### 3. **Chapel** (Parallel Iterators)
Designed for HPC with parallel-by-default iterators:

```chapel
// Automatically parallelizes
forall i in 1..1000 do
  A[i] = compute(i);

// Data parallelism built-in
var A: [1..1000] real;
A = A * 2;  // Automatic parallel execution
```

**Why it works:**
- **Parallel iterators** - default to parallel execution
- **Data locality** - explicit control over distribution
- **PGAS model** - partitioned global address space

**Getting closer:** But still requires programmer to use forall vs for

### 4. **Cilk** (Work Stealing)
Fork-join parallelism with provably efficient scheduling:

```c
int fib(int n) {
  if (n < 2) return n;

  int x = cilk_spawn fib(n-1);  // Parallel
  int y = fib(n-2);
  cilk_sync;

  return x + y;
}
```

**Why it works:**
- **Work stealing** - automatic load balancing
- **Provable bounds** - near-optimal scheduling
- **Composability** - spawn/sync compose well

**Limitation:** Requires explicit spawn annotations

### 5. **NESL** (Nested Data Parallelism)
Automatically parallelizes nested data-parallel operations:

```nesl
{sum(a) : a in partition(data)}  % Automatic parallelism!
```

**Why it works:**
- **Flat data parallelism** - compiler flattens nested structures
- **Cost model** - predictable performance
- **Purely functional** - no side effects

**Closest to automatic:** Operations are parallel by default

## Design Principles for Auto-Parallel Languages

### Principle 1: **Make Independence Explicit**
```
// Bad: Implicit dependencies
[process data]  // Unknown if independent

// Good: Explicit independence
[parallel work 1]  // Marked as independent
[parallel work 2]
[parallel work 3]
```

### Principle 2: **Immutability by Default**
```
// Bad: Mutable state
let mut x = 0;
threads.map(|t| x += compute(t))  // Race condition!

// Good: Immutable with explicit reduction
let results = threads.map(|t| compute(t));
let x = results.sum();  // Safe parallel + reduce
```

### Principle 3: **Explicit Communication**
```
// Bad: Shared memory
global_state.update()  // Hidden communication

// Good: Message passing
send(result, destination)  // Explicit communication
```

### Principle 4: **Pure Computations**
```
// Bad: Side effects
fn compute(x) {
  write_to_db(x);  // Hidden side effect
  return x * 2;
}

// Good: Pure function
fn compute(x) -> Result {
  return (x * 2, WriteDB(x));  // Explicit effects
}
```

## Designing Parallel Fraglets

### Option 1: **Scoped Regions (Static)**
Add explicit parallel/sequential annotations:

```fraglets
# Declare independent work items
[parallel work 1]
[parallel work 2]
[parallel work 3]

# Matchp rules can still be global
[matchp work dup result]

# System knows these are independent!
```

**How it works:**
- Parser detects `[parallel ...]` prefix
- Routes by work ID automatically
- Guarantees no cross-dependencies

### Option 2: **Functional Fraglets**
Make molecules immutable:

```fraglets
# Old: Molecules consumed
[matchp work result]  # Consumes 'work', creates 'result'

# New: Transform returns new molecules
work(1) -> result(1)  # Functional transformation
```

**How it works:**
- No molecule mutation
- All reactions are pure transformations
- Can parallelize freely

### Option 3: **Type System for Locality**
Annotate molecule types with location:

```fraglets
# Local molecules (region-private)
[local work 1] : Local<Work>

# Global molecules (need synchronization)
[global counter] : Global<Counter>

# System routes Local automatically!
```

**How it works:**
- Type checker ensures Local doesn't escape
- Global requires explicit synchronization
- Compiler can parallelize Local

### Option 4: **Dataflow Dependencies**
Explicit dependency graph:

```fraglets
# Declare data dependencies
work1 : []           # No dependencies
work2 : [work1]      # Depends on work1
work3 : []           # Independent

# System builds dependency graph!
```

**How it works:**
- Static analysis of dependencies
- Parallel execution of independent nodes
- Sequential execution of dependent chains

## The Best Design: Hybrid Approach

Combine multiple strategies:

```fraglets-parallel
# 1. Scoped parallel blocks
parallel {
  [work 1]
  [work 2]
  [work 100]
}

# 2. Pure matchp rules (immutable)
matchp work(N) -> result(N) {
  dup -> step1
  step1 -> result
}

# 3. Explicit global state
global {
  [counter 0]
}

# 4. Barrier synchronization
parallel {
  [work 1]
  [work 2]
}
barrier  # Wait for all work to complete

sequential {
  [sort data]  # Sequential algorithm
}
```

**Advantages:**
- `parallel {}` blocks route by hash automatically
- Pure functions enable safe parallelism
- Global state explicit and limited
- Programmer control when needed

## Real-World Example: TensorFlow

TensorFlow achieves automatic parallelism via:

```python
# Implicitly parallel (dataflow graph)
a = tf.constant([1, 2, 3])
b = tf.constant([4, 5, 6])
c = a + b  # Automatic parallelism!
```

**How:**
1. **Dataflow graph** - operations are nodes, data flows on edges
2. **Static analysis** - no dependencies = parallel
3. **Device placement** - automatic GPU/CPU assignment
4. **Runtime scheduling** - dynamic load balancing

## Recommendation for Fraglets

Add minimal syntax for explicit parallelism:

```fraglets
# Mark independent work with @parallel
@parallel
[work 1]
[work 2]
[work 100]

# Regular molecules (sequential)
[sort 27 numbers]
```

**Implementation:**
- Parser detects `@parallel` prefix
- Routes by hash(molecule_id) automatically
- No changes to runtime needed
- Programmer makes informed choice

This is **honest**: parallelism requires programmer understanding, but syntax makes it easy.

## Summary

**Existing languages that achieve automatic parallelism:**
1. Haskell (purity + laziness)
2. Chapel (parallel iterators)
3. NESL (nested data parallelism)
4. TensorFlow (dataflow graphs)

**Key insight:** True automatic parallelism requires either:
- **Purity** (no side effects) - Haskell
- **Explicit structure** (parallel blocks) - Chapel
- **Dataflow** (explicit dependencies) - TensorFlow

**For fraglets:** Add `@parallel` annotations to mark independent work, maintaining simplicity while enabling programmer control.

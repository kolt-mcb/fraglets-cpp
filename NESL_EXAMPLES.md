# NESL - Nested Data Parallel Language

NESL was developed at Carnegie Mellon University in the 1990s as a research language for automatic parallelism through nested data parallelism.

## Basic Syntax

### Parallel Operations (Automatic!)

```nesl
% Vector addition - automatically parallel
{a + b : a in [1,2,3]; b in [4,5,6]};
-> [5,7,9]

% Map operation - parallel by default
{a * 2 : a in [1,2,3,4,5]};
-> [2,4,6,8,10]

% Filter
{a : a in [1,2,3,4,5] | a > 2};
-> [3,4,5]

% Combined map and filter
{a * a : a in [1,2,3,4,5,6] | even(a)};
-> [4,16,36]
```

### Nested Parallelism

```nesl
% Nested comprehensions - outer AND inner are parallel!
{ {a * b : b in [1,2,3]} : a in [10,20,30] };
-> [[10,20,30], [20,40,60], [30,60,90]]

% Matrix multiplication (automatically parallel in both dimensions)
function matrix_mult(A, B) =
  { {sum({a * b : a in row; b in col}) : col in transpose(B)}
    : row in A };
```

### Aggregate Operations

```nesl
% Sum - parallel reduction
sum([1,2,3,4,5]);
-> 15

% Max element
max_val([3,1,4,1,5,9]);
-> 9

% All comparisons happen in parallel
{a < 5 : a in [1,2,3,4,5,6,7]};
-> [T,T,T,T,T,F,F]
```

## Real Examples

### Quicksort (Parallel!)

```nesl
function quicksort(s) =
  if #s <= 1 then s
  else
    let pivot = s[#s/2];
    let lesser = {x : x in s | x < pivot};
    let equal  = {x : x in s | x == pivot};
    let greater = {x : x in s | x > pivot};
    in quicksort(lesser) ++ equal ++ quicksort(greater);
```

**What's automatic:**
- Partitioning into lesser/equal/greater is parallel
- Both recursive calls can run in parallel
- No explicit spawn/fork needed!

### Word Count (MapReduce)

```nesl
function word_count(documents) =
  % Map phase - parallel over all documents
  let words = flatten({tokenize(doc) : doc in documents});

  % Group by word - parallel
  let grouped = group_by_key(words);

  % Reduce phase - parallel over all unique words
  let counts = {(word, #instances) : (word, instances) in grouped};
  in counts;
```

### Prime Numbers (Sieve of Eratosthenes)

```nesl
function primes(n) =
  let candidates = [2:n];  % Range from 2 to n
  in sieve(candidates);

function sieve(s) =
  if #s == 0 then []
  else
    let p = s[0];  % First element is prime
    let rest = {x : x in s | (x mod p) != 0};  % Filter multiples (parallel!)
    in [p] ++ sieve(rest);

% Usage
primes(100);
-> [2,3,5,7,11,13,17,19,23,29,31,37,41,43,47,53,59,61,67,71,73,79,83,89,97]
```

### K-Means Clustering

```nesl
function kmeans(points, k, iterations) =
  let initial_centers = take(k, points);
  in iterate(iterations, initial_centers, points);

function iterate(n, centers, points) =
  if n == 0 then centers
  else
    % Assign each point to nearest center (parallel!)
    let assignments = {nearest(p, centers) : p in points};

    % Group points by cluster (parallel!)
    let clusters = partition(points, assignments, k);

    % Compute new centers (parallel over clusters!)
    let new_centers = {centroid(cluster) : cluster in clusters};

    in iterate(n-1, new_centers, points);

function nearest(point, centers) =
  % Find closest center (parallel comparison)
  min_index({distance(point, c) : c in centers});

function centroid(points) =
  % Average position (parallel sum)
  let n = #points;
  in {sum(coords) / n : coords in transpose(points)};
```

## Key Features

### 1. Comprehensions are Parallel by Default

```nesl
% This is AUTOMATICALLY parallel - no annotation needed!
{expensive_function(x) : x in huge_list};
```

### 2. Flattening Enables Nested Parallelism

```nesl
% Nested structure
[[1,2,3], [4,5], [6,7,8,9]]

% Flatten to execute in parallel
flatten([[1,2,3], [4,5], [6,7,8,9]]);
-> [1,2,3,4,5,6,7,8,9]

% Then apply operations in parallel
{x * 2 : x in flatten(nested_list)};
```

### 3. Aggregate Operations

```nesl
% All parallel reductions
sum([1,2,3,4,5])           % -> 15
product([1,2,3,4,5])       % -> 120
max_val([3,1,4,1,5,9])     % -> 9
min_val([3,1,4,1,5,9])     % -> 1
any([F,F,T,F])             % -> T
all([T,T,T,F])             % -> F
```

### 4. Scan Operations (Parallel Prefix)

```nesl
% Plus-scan (parallel prefix sum)
plus_scan([1,2,3,4,5]);
-> [0,1,3,6,10]  % [0, 0+1, 0+1+2, 0+1+2+3, 0+1+2+3+4]

% Max-scan
max_scan([3,1,4,1,5,9,2]);
-> [3,3,4,4,5,9,9]
```

## Comparison: NESL vs Other Languages

### Vector Addition

**NESL (automatic parallel):**
```nesl
{a + b : a in vec1; b in vec2};
```

**Chapel (explicit parallel):**
```chapel
forall (a, b) in zip(vec1, vec2) do
  a + b;
```

**Haskell (potentially parallel):**
```haskell
zipWith (+) vec1 vec2  -- Can be parallelized
```

**Fraglets (manual):**
```fraglets
@parallel {
  [add 1 4]
  [add 2 5]
  [add 3 6]
}
```

### Matrix Multiplication

**NESL (fully automatic):**
```nesl
{{sum({a*b : a in row; b in col}) : col in transpose(B)} : row in A};
```

**NumPy (library parallelism):**
```python
A @ B  # Internally parallel via BLAS
```

**TensorFlow (dataflow parallelism):**
```python
tf.matmul(A, B)  # Automatic device placement
```

## Why NESL Achieves Automatic Parallelism

### 1. Pure Functional (No Side Effects)
```nesl
% This function is pure - can run anywhere
function double(x) = x * 2;

% Safe to parallelize - no shared state
{double(x) : x in [1,2,3,4,5]};
```

### 2. No Mutation
```nesl
% Can't do this - no mutation!
% x = x + 1;  % ERROR!

% Instead, create new values
let new_x = x + 1;
```

### 3. Structured Parallelism (Only Comprehensions)
```nesl
% Only parallel construct - comprehension
{expression : variable in sequence}

% No threads, no spawn, no locks
% Limited expressiveness = automatic analysis
```

### 4. Cost Model
NESL has a provable cost model - you can predict performance:
- Step complexity: O(work / processors)
- Space complexity: O(work)

## Limitations

### 1. No Imperative Code
```nesl
% Can't do loops
% for i in 1..100 do ...  % NOT POSSIBLE

% Must use recursion
function loop(i, n) =
  if i > n then []
  else [compute(i)] ++ loop(i+1, n);
```

### 2. No I/O During Computation
```nesl
% Can't do this
% {print(x); compute(x) : x in data};  % ERROR

% I/O separate from computation
let results = {compute(x) : x in data};
print(results);  % After computation
```

### 3. Limited Data Structures
```nesl
% Only sequences (vectors)
% No hash tables, trees, graphs (directly)

% Must encode as sequences
% Graph as adjacency list: [[1,2,3], [2,3], [1], []]
```

### 4. Not Widely Used
- Research language (1990s)
- Influenced others (Fortress, Data Parallel Haskell)
- Never reached production use

## What NESL Teaches Us

For automatic parallelism, you need:

1. **Purity** - No side effects
2. **Structured parallelism** - Limited constructs
3. **Data parallelism** - Operations on collections
4. **No mutation** - Immutable data

**Trade-off:** Expressiveness for automatic parallelism

## Could Fraglets Be Like NESL?

### If Fraglets Were Pure:

```fraglets-pure
% Pure transformations (like NESL comprehensions)
{[result N] : [work N] in molecules};

% Automatically parallel!
```

**Problem:** Fraglets reactions have side effects:
- Consume molecules
- Create new molecules
- Modify global state

### Hybrid Approach:

```fraglets
% Mark pure transformations
@pure_parallel {
  {[result N] : [work N] in molecules}
}

% Traditional reactions for stateful stuff
@sequential {
  [matchp work ...complex logic...]
}
```

## Conclusion

**NESL achieves automatic parallelism by:**
- Being purely functional
- Limiting expressiveness to data-parallel patterns
- Having no side effects or mutation

**For fraglets:** We can't be NESL because reactions inherently have side effects (consume/create molecules). Our `@parallel` annotation approach is more honest and practical.

But NESL's lesson is valuable: **automatic parallelism requires giving up something** (in NESL's case, imperative programming and side effects).

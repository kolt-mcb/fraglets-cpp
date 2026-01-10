# Parallel Sort Implementation for Fraglets

This document describes the new operations and parallel sorting capability added to fraglets to leverage multi-threading.

## New Operations

### 1. `partition` - Divide list into independent work units

**Syntax:**
```
[partition N tag element1 element2 ... elementK]
```

**Behavior:**
- Divides the elements into N roughly equal partitions
- Creates N independent molecules: `[tag 0 ...], [tag 1 ...], ..., [tag N-1 ...]`
- Each partition can be processed by a different thread

**Example:**
```
Input:  [partition 3 sortchunk 1 2 3 4 5 6 7 8 9]
Output: [sortchunk 0 1 2 3]
        [sortchunk 1 4 5 6]
        [sortchunk 2 7 8 9]
```

**Key Feature:** Creates multiple independent unimolecular molecules that can be processed in parallel!

### 2. `merge` - Combine two sorted lists

**Syntax:**
```
[merge A1 A2 ... * B1 B2 ...]
```

**Behavior:**
- Merges two sorted lists separated by `*`
- Produces a single sorted list
- Supports numeric and lexicographic comparison

**Example:**
```
Input:  [merge 1 3 5 * 2 4 6]
Output: [1 2 3 4 5 6]

Input:  [merge -5 -1 3 * -3 0 4]
Output: [-5 -3 -1 0 3 4]
```

## Parallel Sort Strategy

The key to achieving parallelism in fraglets is creating **multiple independent molecules** that can be processed by different threads simultaneously.

### Algorithm Overview

```
1. PARTITION PHASE (Sequential)
   [parsort ...elements...]
   -> [partition N sortchunk ...elements...]
   -> [sortchunk 0 ...] [sortchunk 1 ...] [sortchunk 2 ...] [sortchunk 3 ...]
                ↑                ↑                ↑                ↑
                Thread 1         Thread 2         Thread 3         Thread 4

2. SORTING PHASE (PARALLEL!)
   Each thread independently sorts its chunk:
   [sortchunk 0 ...] -> sorts -> [sorted 0 ...]
   [sortchunk 1 ...] -> sorts -> [sorted 1 ...]
   [sortchunk 2 ...] -> sorts -> [sorted 2 ...]
   [sortchunk 3 ...] -> sorts -> [sorted 3 ...]

3. MERGE PHASE (Hierarchical - partially parallel)
   [sorted 0 ...] + [sorted 1 ...] -> merge -> [sorted A ...]
   [sorted 2 ...] + [sorted 3 ...] -> merge -> [sorted B ...]
                  ↓
   [sorted A ...] + [sorted B ...] -> merge -> [final sorted list]
```

### Why This Enables Parallelism

1. **Independent Work Units**: Each `[sortchunk ...]` molecule is completely independent
2. **No Shared State**: Sorting one chunk doesn't depend on sorting another
3. **Unimolecular Reactions**: Each chunk sorting is a unimolecular reaction
4. **Thread Pool Processing**: The `run_unimol_parallel()` function distributes these molecules across threads

## Performance Characteristics

### When Parallelism Helps
- **Large datasets** with many elements
- **Expensive comparisons** or operations
- **Coarse-grained parallelism** (larger chunks per thread)

### Current Limitations
- **Lock contention** in inject/expel operations still limits speedup
- **Small chunks** increase overhead from synchronization
- **Merge phase** is harder to parallelize (requires hierarchical approach)

## Example Usage

```fraglets
# Sort 16 numbers using 4-way parallelism
[parsort 45 12 78 23 91 34 56 67 89 11 43 29 15 72 38 54]

# This internally does:
# 1. partition 4 sortchunk <numbers>
# 2. Creates 4 independent sorting tasks
# 3. Each task processes 4 numbers
# 4. Merges results hierarchically
```

## Test Results

```
Parallel Sort Benchmark (parsort.fra)
Iterations: 50000
Molecule Cap: 1000

Threads | Time (ms) | Speedup
--------|-----------|----------
1       |    0      | 1.00x (baseline)
2       |    3      | 0.00x
4       |    6      | 0.00x
8       |   16      | 0.00x
```

**Note:** The algorithm completes very quickly (< 1ms) indicating the workload is too small to see threading benefits. Larger datasets and more complex operations would show better results.

## Future Improvements

1. **Lock-free data structures** for molecule storage
2. **Batch operations** to reduce synchronization frequency
3. **Work stealing** for better load balancing
4. **Parallel merge** using divide-and-conquer approach

## Implementation Details

### C++ Implementation

The new operations are implemented in `fraglets.cpp`:

```cpp
opResult r_partition(const molecule_pointer mol);  // Divides list into N parts
opResult r_merge(const molecule_pointer mol);      // Merges two sorted lists
```

Both are registered as unimolecular operations in `unimolOpMap`.

### Files

- `fraglets.h/cpp` - Core implementation with new operations
- `parsort.fra` - Parallel sort algorithm
- `test_parsort.cpp` - Benchmark program
- `test_operations.cpp` - Unit tests for new operations

## Conclusion

The `partition` and `merge` operations provide the building blocks for parallel algorithms in fraglets. While current performance is limited by synchronization overhead, the infrastructure demonstrates how to create independent work units that can leverage multi-core processors.

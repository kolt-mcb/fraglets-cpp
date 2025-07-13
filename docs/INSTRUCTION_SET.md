# Fraglets Instruction Summary

This document summarizes the instruction set used by the Fraglets engine. It is derived from the `frag-instrset-20070924.txt` reference and the example programs included in this repository.

Fraglets are lists of tokens. The first token of a list acts as the instruction tag. Rules consume the instruction tag and transform the remaining tokens. When two fraglets "match," the rule bodies can combine or generate new fraglets.

## 1. Core Instructions

| Instruction | Effect |
|-------------|--------|
| `dup` | `[dup t a tail]` → `[t a a tail]` – duplicate a single symbol |
| `exch` | `[exch t a b tail]` → `[t b a tail]` – swap two tags |
| `fork` | `[fork a b tail]` → `[a tail]`, `[b tail]` – copy a fraglet with two different prefixes |
| `match` | `[match a tail1]` and `[a tail2]` → `[tail1 tail2]` – concatenate fraglets when their first symbols match |
| `matchp` | `[matchp a tail1]` and `[a tail2]` → `[matchp a tail1]`, `[tail1 tail2]` – catalytic match; the rule persists |
| `nop` | `[nop tail]` → `[tail]` – no operation |
| `nul` | `[nul tail]` → `[]` – destroy a fraglet |
| `pop2` | `[pop2 h t a b tail]` → `[h a]`, `[t b tail]` – pop the head of a list into a separate fraglet |
| `split` | `[split seq1 * seq2]` → `[seq1]`, `[seq2]` – break a fraglet at the first `*` |

## 2. Communication and Timing

| Instruction | Effect |
|-------------|--------|
| `broadcast` | `[broadcast seg tail]` → `n[tail]` – copy `tail` to all neighbours on segment `seg` |
| `delay` | `[delay n tail]` → `[tail]` after waiting `n` cycles |
| `send` | `[send seg dest tail]` → `dest[tail]` – send to a node on segment `seg`; special `stdout` and `stderr` destinations print `tail` |

## 3. Logic and Arithmetic

| Instruction | Effect |
|-------------|--------|
| `abs` | `[abs tag n tail]` → `[tag |n| tail]` |
| `div` | `[div tag n1 n2 tail]` → `[tag n1/n2 tail]`; removed if `n2` is zero |
| `empty` | `[empty yes no tail]` → `[yes]` if `tail` is empty, else `[no tail]` |
| `eq` | `[eq yes no n m tail]` → `[yes n m tail]` if equal, else `[no n m tail]` |
| `length` | `[length t1 tail]` → `[t1 |tail| tail]` |
| `lt` | `[lt yes no n m tail]` → `[yes n m tail]` if `n < m`, else `[no n m tail]` |
| `mod` | `[mod tag n1 n2 tail]` → `[tag (n1 % n2) tail]` |
| `mult` | `[mult tag n1 n2 tail]` → `[tag (n1 * n2) tail]` |
| `pow` | `[pow tag n1 n2 tail]` → `[tag (n1 ^ n2) tail]` |
| `sub` | `[sub tag n1 n2 tail]` → `[tag (n1 - n2) tail]` |
| `sum` | `[sum tag n1 n2 tail]` → `[tag (n1 + n2) tail]` |

## 4. Reserved Keywords

`*` – used by `split` to mark the separation point  
`stdout`, `stderr` – pseudo destinations for `send`  
`stdin`, `out`, `_`, `__` – reserved for future extensions

## 5. Experimental Extensions

| Instruction | Effect |
|-------------|--------|
| `anycast` | `[anycast seg tail]` → `n[tail]` – copy to at most one neighbour |
| `copy` | `[copy tail]` → `[tail]2` |
| `pop` | `[pop h a b c]` → `[h b c]` (proposed rename to `tail`) |
| `printsym` | `[printsym sym tail]` → `[tail]` while printing `sym` |
| `wait` | `[wait a b c]` → `[a b c]` after 10 cycles; prefer `delay` |
| `splitat` | `[splitat | a b c | x y z]` → `[a b c]`, `[x y z]` – split at a marker |
| `newnode` | `m[newnode ch n tail]` → `a n ch`, `n[tail]` – create child node |
| `inject` | `n1[inject n2 tail]` → `n2[tail]` (child) |
| `expel` | `n2[expel tail]` → `n1[tail]` (parent) |
| `newname` | `[newname tag s1 s2 tail]` → `[tag s1s2 tail]` |

Deprecated instruction: `diff` (use `sub` and `abs` instead).

## Example – Selection Sort

The repository includes `sort.fra`, which implements a selection sort using these instructions. Key rules:

```fraglets
[matchp sort empty finish continue]
[matchp continue split remain * getmin]
[matchp min split match remain sort * split match sorted match tosorted sorted * tosorted]
```

The `getmin` function repeatedly uses `matchp`, `lt`, and `pop2` to find the smallest number and store the rest in a `remain` list.

To run the example via Python:

```bash
python3 -m pip install .  # build the cFraglets module
python3 test_sort.py
```

This parses `sort.fra`, executes the fraglet engine for a fixed number of iterations, and verifies that the resulting list is sorted.

Refer to `README.md` for build instructions and `test_sort.py` for a complete usage example.

## Experimental QuickSort

`quicksort.fra` demonstrates how the same primitives can implement a recursive
quicksort. The program uses persistent rules to split the list around a pivot,
recursively sort the `less` and `greater` partitions and finally merge the
results. It serves as a more advanced example of fraglet programming but has not
been extensively tuned.


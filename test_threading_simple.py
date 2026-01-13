#!/usr/bin/env python3
"""
Simple test to verify multi-threading works without crashing.
"""

import fraglets
import time

def test_threading():
    print("Testing multi-threaded execution...")
    print("="*60)

    for num_threads in [1, 2, 4, 8]:
        print(f"\nTesting with {num_threads} threads:")

        frag = fraglets.fraglets()

        # Parse sort rules
        with open('sort.fra', 'r') as f:
            for line in f:
                line = line.strip()
                if line and not line.startswith('#') and not line.startswith('[psort 203'):
                    frag.parse(line)

        # Small test
        frag.parse("[psort 10 -5 20 -15 8]")

        start = time.perf_counter()
        if num_threads == 1:
            frag.run(1000, 5000, quiet=True)
        else:
            frag.run_parallel(1000, 5000, quiet=True, threads=num_threads)
        elapsed = time.perf_counter() - start

        print(f"  Completed in {elapsed*1000:.3f}ms, {frag.iter} iterations")
        print(f"  ✓ No crash!")

    print("\n" + "="*60)
    print("Threading test PASSED - all thread counts work!")

if __name__ == "__main__":
    test_threading()

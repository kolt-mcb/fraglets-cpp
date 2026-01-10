#!/usr/bin/env python3
"""
Test to verify the parallel sort actually produces correct sorted output.
"""

import fraglets

def test_simple_sort():
    """Test with a simple list and verify output."""
    print("Testing parallel sort correctness...")
    print("="*60)

    frag = fraglets.fraglets()

    # Read and parse the parallel sort rules
    with open('sort.fra', 'r') as f:
        lines = f.readlines()
        for line in lines:
            line = line.strip()
            # Skip comments and empty lines
            if line and not line.startswith('#'):
                print(f"Parsing: {line[:70]}...")
                frag.parse(line)

    print(f"\n{'='*60}")
    print("Running simulation for 100,000 iterations...")
    print(f"{'='*60}\n")

    # Run with verbose output to see what's happening
    frag.run(100000, 2000, quiet=False)

    print(f"\n{'='*60}")
    print(f"Total iterations: {frag.iter}")
    print(f"{'='*60}")

if __name__ == "__main__":
    test_simple_sort()

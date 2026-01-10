#!/usr/bin/env python3
import subprocess
import re

# Run sort
result = subprocess.run(
    ['./target/release/fraglets', 'sort.fra', '--iterations', '1000000'],
    capture_output=True,
    text=True,
    timeout=30
)

# Extract numbers from output
numbers = []
for line in result.stdout.split('\n'):
    # Look for lines like ["123"] or ["-456"]
    match = re.match(r'\s*\["(-?\d+)"\]', line)
    if match:
        numbers.append(int(match.group(1)))

print(f"Found {len(numbers)} numbers in output")
print(f"Numbers: {numbers[:20]}...")  # First 20

if len(numbers) > 1:
    is_sorted = all(numbers[i] <= numbers[i+1] for i in range(len(numbers)-1))
    print(f"\nIs sorted: {is_sorted}")
    if not is_sorted:
        print("FAIL: Numbers are not sorted!")
        print(f"First unsorted pair: {[(numbers[i], numbers[i+1]) for i in range(len(numbers)-1) if numbers[i] > numbers[i+1]][:5]}")
else:
    print("Not enough numbers to verify sorting")

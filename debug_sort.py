#!/usr/bin/env python3
# Simulate the sort algorithm to understand the flow

molecules = [
    ["matchp", "sort", "empty", "finish", "continue"],
    ["sort", "5", "3", "8"],
]

def matchp_react(matchp_rule, target):
    """Simulate matchp reaction"""
    if len(matchp_rule) < 2:
        return None

    pattern = matchp_rule[1]  # What to match
    transform = matchp_rule[2] if len(matchp_rule) > 2 else None
    rest_of_rule = matchp_rule[3:]

    if target[0] == pattern:
        # Match! Create result
        result = [transform] + rest_of_rule + target[1:]
        return [matchp_rule, result]  # Rule persists + result
    return None

def empty_react(mol):
    """Simulate empty operation"""
    if mol[0] == "empty" and len(mol) > 3:
        return [mol[2:]]  # Remove "empty" and next symbol
    return None

# Step 1: matchp reacts
print("Step 1: matchp + sort")
result = matchp_react(molecules[0], molecules[1])
if result:
    print(f"  Reactants: {molecules[0]} + {molecules[1]}")
    print(f"  Products: {result}")
    molecules = result
else:
    print("  No reaction!")

print(f"\nCurrent molecules: {molecules}")

# Step 2: empty should react
print("\nStep 2: empty operation")
for i, mol in enumerate(molecules):
    result = empty_react(mol)
    if result:
        print(f"  Reactant: {mol}")
        print(f"  Product: {result}")
        molecules = molecules[:i] + result + molecules[i+1:]
        break

print(f"\nFinal molecules: {molecules}")

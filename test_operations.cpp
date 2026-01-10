#include "fraglets.h"
#include <iostream>

std::string alphabet = {"abcdtuvxz"};

int main() {
    std::cout << "Testing new partition and merge operations" << std::endl;
    std::cout << "==========================================" << std::endl;

    fraglets frag;

    // Test partition operation
    std::cout << "\nTest 1: Partition operation" << std::endl;
    std::cout << "Input: [partition 3 chunk 1 2 3 4 5 6 7 8 9]" << std::endl;
    frag.parse("partition 3 chunk 1 2 3 4 5 6 7 8 9");
    frag.run(10, 100, false);  // Run with output to see what happens

    std::cout << "\n==========================================\n" << std::endl;

    // Test merge operation
    fraglets frag2;
    std::cout << "Test 2: Merge operation" << std::endl;
    std::cout << "Input: [merge 1 3 5 * 2 4 6]" << std::endl;
    frag2.parse("merge 1 3 5 * 2 4 6");
    frag2.run(10, 100, false);

    return 0;
}

#include "fraglets.h"
#include <iostream>

std::string alphabet = {"abcdtuvxz"};

int main() {
    std::cout << "Verifying that parsort actually works..." << std::endl;
    std::cout << "========================================" << std::endl;

    fraglets frag;
    frag.interpret("parsort_xlarge.fra");

    std::cout << "Running 100 iterations with output enabled..." << std::endl;
    frag.run(100, 10000, false);  // quiet=false to see output

    return 0;
}

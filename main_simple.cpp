#include "fraglets.h"
#include <chrono>
#include <iostream>
#include <iomanip>

std::string alphabet = {"abcdtuvxz"};

void setup_fraglets(fraglets& frag) {
    symbol mol = "fork nop z match z split match z fork fork fork nop z * split match z fork fork fork nop z * copy z";
    frag.parse(mol);

    for (int i = 0; i< 50; i++){
        frag.parse(mol);
        frag.parse("z");
    }

    symbol mol2 = "perm z ";
    std::string::iterator alphaIt2;
    std::unordered_set<std::string>::iterator uIt;

    for (alphaIt2 = alphabet.begin();alphaIt2!=alphabet.end();alphaIt2++){
        symbol newMol = mol2 + *alphaIt2;
        frag.parse(newMol);
        newMol = mol2 + " z " + *alphaIt2;
        frag.parse(newMol);
    }

    for (uIt = unimolTags.begin();uIt!=unimolTags.end();uIt++){
        symbol newMolTag = mol2  + *uIt;
        frag.parse(newMolTag);
        newMolTag = mol2 + " z " +*uIt;
        frag.parse(newMolTag);
    }

    for (alphaIt2 = alphabet.begin();alphaIt2!=alphabet.end();alphaIt2++){
        symbol newMol2 = mol2 + " match " + *alphaIt2;
        frag.parse(newMol2);
        newMol2 = mol2 + " z " + "match " + *alphaIt2;
        frag.parse(newMol2);

        symbol newMol3 = mol2 + " matchp " + *alphaIt2;
        frag.parse(newMol3);
        newMol3 = mol2 + " z " + "matchp " + *alphaIt2;
        frag.parse(newMol3);
    }
}

int main() {
    const int iterations = 10000;
    const int molCap = 200;
    const int num_threads = 8;

    std::cout << "========================================" << std::endl;
    std::cout << "Testing with " << num_threads << " threads" << std::endl;
    std::cout << "========================================" << std::endl;

    fraglets frag;
    setup_fraglets(frag);

    std::cout << "Starting benchmark..." << std::endl;
    auto start = std::chrono::high_resolution_clock::now();
    frag.run(iterations, molCap, true, true, num_threads);
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    std::cout << "Completed in " << duration.count() << " ms" << std::endl;
    std::cout << "About to exit..." << std::endl;

    return 0;
}

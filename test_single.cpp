#include "fraglets.h"
#include <chrono>
#include <iostream>

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
    std::cout << "Testing single-threaded version..." << std::endl;

    fraglets frag;
    setup_fraglets(frag);

    std::cout << "Running 10000 iterations..." << std::endl;
    auto start = std::chrono::high_resolution_clock::now();
    frag.run(10000, 200, true, false, 1);  // parallel=false
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    std::cout << "Completed in " << duration.count() << " ms" << std::endl;

    return 0;
}

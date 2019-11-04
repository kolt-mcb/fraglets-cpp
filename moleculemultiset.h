#include <unordered_map>
#include <vector>
#include <unordered_set>
#include <set>
#include <iostream>
#include <random>

typedef std::string molecule;
typedef std::vector<molecule> moleculeVector;
typedef std::unordered_multiset<molecule>   unorderedMultiset;


class moleculeMultiset {

    public:
        void inject(const molecule& mol, int mul);
        int expel(const molecule& mol, int mult);
        molecule expelrnd(const molecule& mol);
        molecule rndMol();
        molecule expelrnd();
        int mult(molecule mol);
        int mult();
        unorderedMultiset multiset;

};
#include <unordered_map>
#include <vector>
#include <unordered_set>
#include <set>
#include <iostream>
#include <random>

typedef std::string molecule;
typedef std::vector<molecule> moleculeVector;
typedef std::unordered_map<molecule,int>   unorderedMultiset;


class moleculeMultiset {

    public:
        void inject(molecule molecule, int mul);
        int expel(molecule mol, int mult);
        molecule expelrnd(molecule mol);
        molecule rndMol();
        molecule expelrnd();
        unorderedMultiset multiset;

};
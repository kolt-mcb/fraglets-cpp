#include <unordered_map>
#include <vector>
#include <unordered_set>
#include <set>
#include <iostream>
#include <random>





int rand_between(int begin, int end);

typedef std::string symbol;
typedef std::vector<symbol> molecule;
typedef std::vector<molecule> moleculeVector;
typedef std::multiset<molecule>   unorderedMultiset;


class moleculeMultiset {

    public:
        // moleculeMultiset();
        void inject(molecule& mol, int mul);
        int expel(molecule& mol, int mult);
        molecule expelrnd();
        molecule rndMol();
        int mult(molecule mol);
        int mult();
        unorderedMultiset multiset;
};
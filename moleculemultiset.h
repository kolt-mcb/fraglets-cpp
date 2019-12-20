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
typedef std::unordered_multiset<const molecule*>   unorderedMultiset;


class moleculeMultiset {

    public:
        // moleculeMultiset();
        void inject(const molecule* mol, int mul);
        int expel(const molecule* mol, int mult);
        const molecule& expelrnd();
        const molecule* rndMol();
        int mult(molecule mol);
        int mult();
        unorderedMultiset multiset;
};
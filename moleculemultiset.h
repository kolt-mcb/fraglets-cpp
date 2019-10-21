#include <vector>
#include <unordered_set>
#include <set>

typedef std::vector<std::string> molecule;
typedef std::unordered_multiset<molecule*>   unorderedMultiset;


class moleculeMultiset {

    public:
        void inject(molecule molecule, int mul);
        int expel(molecule mol, int mult);
        molecule expelrnd(molecule mol);
        molecule rndMol();
        molecule expelrnd();
        unorderedMultiset multiset;

};
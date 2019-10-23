
#include "moleculemultiset.h"


typedef std::unordered_map<std::string,moleculeMultiset*>  keyMultisetMap;


class keyMultiset  {
    
    public:
        void inject(std::string key, molecule molecule, int mult=1);
        void expel(std::string key, molecule mol, int mult=1);
        molecule rndmol(std::string key);
        molecule expelrnd(std::string key);
        int mult(molecule molecule);
        int multk(std::string key);
        int nspecies();
        keyMultisetMap keyMap;

    
};
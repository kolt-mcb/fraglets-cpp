
#include "moleculemultiset.h"


typedef std::unordered_map<symbol,moleculeMultiset*>  keyMultisetMap;


class keyMultiset  {
    
    public:
        int total = 0;
        void inject(std::shared_ptr<symbol> key, std::shared_ptr<molecule> mol, int mult=1);
        void expel(symbol key, std::shared_ptr<molecule> mol, int mult=1);
        std::shared_ptr<molecule> rndmol(symbol key);
        std::shared_ptr<molecule> expelrnd(symbol key);
        int mult(const std::shared_ptr<molecule> mol);
        int multk(symbol key);
        int nspecies();
        keyMultisetMap keyMap;

    
};
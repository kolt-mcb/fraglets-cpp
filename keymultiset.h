
#include "moleculemultiset.h"


typedef std::unordered_map<symbol,moleculeMultiset*>  keyMultisetMap;


class keyMultiset  {


    public:
        std::atomic<int> total{0};
        void inject(std::shared_ptr<symbol> key, const molecule_pointer mol, int mult=1);
        void expel(symbol key,const molecule_pointer mol, int mult=1);
        const molecule_pointer rndmol(symbol key);
        const molecule_pointer expelrnd(symbol key);
        int mult(molecule_pointer& mol);
        int multk(symbol key);
        int nspecies();
        keyMultisetMap keyMap;
        mutable std::mutex mtx;  // Mutex for thread-safe operations on keyMap


};
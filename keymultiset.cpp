#include "keymultiset.h"


class keyMultiset {
    keyMultisetMap keyMap;

    keyMultiset(){
        keyMap = keyMultisetMap();
    }

    void inject(std::string key, molecule molecule, int mult=1){
        if (key.empty() or molecule.empty()){return;}
        keyMultisetMap::iterator it = this->keyMap.find(key);
        moleculeMultiset mset;
        if (it == this->keyMap.end()){
            mset = moleculeMultiset();
            this->keyMap[key] = &mset;
        }else{
            mset = *it->second;
        }
        mset.inject(molecule,mult);

    }

    molecule expel(std::string key, molecule molecule, int mult){

    }
    molecule rndmol(std::string key){
        
    }
    molecule expelrnd(std::string key);
    int mult(std::string molecule);
    int multk(std::string key);
    int nspecies();
}
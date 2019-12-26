#include "keymultiset.h"



void keyMultiset::inject(std::string key, molecule& mol, int mult){
    if ((key.empty()) or (mol.empty())){return;}
    keyMultisetMap::iterator it = this->keyMap.find(key);
    moleculeMultiset* mset;
    if (it == this->keyMap.end()){
        mset = new moleculeMultiset();
        this->keyMap[key] = mset;
    }else{
        mset = it->second;
    }    

    mset->inject(mol,mult);
    this->total += mult;
}

void keyMultiset::expel(std::string key, molecule& mol, int mult){
    if ((key.empty()) or (mult < 0)){ return;}
    else{
        keyMultisetMap::iterator it = this->keyMap.find(key);
        if (it != this->keyMap.end()){
            moleculeMultiset* mset =  it->second;
            int total = mset->expel(mol,mult);
            this->total -= total;
        }
    }
}

molecule keyMultiset::rndmol(std::string key){
    keyMultisetMap::iterator it = this->keyMap.find(key);
    // if (it != this->keyMap.end()){
        moleculeMultiset mset = *it->second;
        molecule mol = mset.rndMol();
        return mol;
    // }
}

molecule keyMultiset::expelrnd(std::string key){
    molecule mol = this->rndmol(key);
    this->expel(key,mol);
    return mol;
};

// int mult(std::string molecule){

// };
int keyMultiset::multk(std::string key){
    // moleculeMultiset m = *this->keyMap[key];
    // std::cout << key << " while\n" ;
    keyMultisetMap::iterator it = this->keyMap.find(key);
    if (it!=this->keyMap.end()){
        moleculeMultiset mset =  *it->second;

        return mset.mult();
    }else
    {   
        return 0;
    }
    // return m.mult();
    
};
int nspecies();


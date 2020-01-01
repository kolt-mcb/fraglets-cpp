#include "keymultiset.h"



void keyMultiset::inject(std::shared_ptr<symbol> key, std::shared_ptr<molecule> mol, int mult){
    if ((key->empty()) or (mol->empty())){return;}
    keyMultisetMap::iterator it = this->keyMap.find(*key);
    moleculeMultiset* mset;
    if (it == this->keyMap.end()){
        mset = new moleculeMultiset();
        this->keyMap[*key] = mset;
    }else{
        mset = it->second;
    }    

    mset->inject(mol,mult);
    this->total += mult;
}

void keyMultiset::expel(symbol key, std::shared_ptr<molecule> mol, int mult){
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

std::shared_ptr<molecule> keyMultiset::rndmol(symbol key){
    keyMultisetMap::iterator it = this->keyMap.find(key);
    // if (it != this->keyMap.end()){
        moleculeMultiset mset = *it->second;
        std::shared_ptr<molecule> mol = mset.rndMol();
        return mol;
    // }
}

std::shared_ptr<molecule> keyMultiset::expelrnd(symbol key){
    std::shared_ptr<molecule> mol = this->rndmol(key);
    this->expel(key,mol);
    return mol;
};

// int mult(std::shared_ptr<symbol> molecule){

// };
int keyMultiset::multk(symbol key){
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


#include "keymultiset.h"



void keyMultiset::inject(std::shared_ptr<symbol> key,const molecule_pointer  mol, int mult){
    if ((key->empty()) or (mol->vector.empty())){return;}
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

void keyMultiset::expel(symbol key,const molecule_pointer mol, int mult){
    if ((key.empty()) or (mult < 0)){ return;}
    else{
        keyMultisetMap::iterator it = this->keyMap.find(key);
        if (it != this->keyMap.end()){
            moleculeMultiset* mset =  it->second;
            int total = mset->expel(mol,mult);
            this->total -= total;
        }
        else{
            std::cout<< "error expel\n";
            exit(0);
        }
    }
}

const molecule_pointer keyMultiset::rndmol(symbol key){
    keyMultisetMap::iterator it = this->keyMap.find(key);
    // if (it != this->keyMap.end()){
        moleculeMultiset mset = *it->second;
        const molecule_pointer mol = mset.rndMol();
        return mol;
    // }
}

const molecule_pointer keyMultiset::expelrnd(symbol key){
    const molecule_pointer mol = this->rndmol(key);
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


#include "keymultiset.h"




// keyMultiset::keyMultiset(){
//     keyMultisetMap keyMap = keyMultisetMap();
// }

void keyMultiset::inject(std::string key, molecule mol, int mult){
    if (key.empty() or mol.empty()){return;}
    keyMultisetMap::iterator it = this->keyMap.find(key);
    moleculeMultiset mset;
    if (it == this->keyMap.end()){
        mset = moleculeMultiset();
        this->keyMap[key] = &mset;
    }else{
        mset = *it->second;
    }
    mset.inject(mol,mult);

}

void keyMultiset::expel(std::string key, molecule mol, int mult){
    if (key.empty() or mult < 0){ return;}
    else{
        keyMultisetMap::iterator it = this->keyMap.find(key);
        if (it != this->keyMap.end()){
            moleculeMultiset mset =  *it->second;
            int total = mset.expel(mol,mult);
            }
    }
}

molecule keyMultiset::rndmol(std::string key){
    keyMultisetMap::iterator it = this->keyMap.find(key);
    if (it != this->keyMap.end()){
        moleculeMultiset mset = *it->second;
        mset.rndMol();
    }
}
molecule keyMultiset::expelrnd(std::string key){
    molecule mol;
    return mol;
};
int mult(std::string molecule);
int multk(std::string key);
int nspecies();


//   def rndmol( self, key ):
//         """ peek at a random molecule with given key, without removing it """
//         if (key not in self.keymset): return ''
//         return self.keymset[key].rndmol()
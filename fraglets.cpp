#include "fraglets.h"
#include <algorithm>


void fraglets::inject(molecule molecule,int mult=1){
    if (molecule.empty() | mult < 1){return;}
    if (molecule.size() >1){
        if (this->isbimol(molecule)){
                std::string key = molecule[1];
                // could check for invalid fraglets here.
                this->active.inject(key,molecule,mult);
                this->idle = false;
            }
        else if (this->isunimol(molecule)){
            for (int i = 0; i<mult;i++){
                this->unimol.inject(molecule,mult);
            }
        }
        else{
            std::string key = molecule[1];
            // could check for invalid fraglets here.
            this->passive.inject(key,molecule,mult);
            this->idle = false;
        }
    }
}   
bool fraglets::isbimol(molecule molecule){
    if (molecule.empty()){ return false;}
    std::string head = molecule[0];
    return head == "match" | head == "matchp";
}
bool fraglets::isunimol(molecule molecule){
    if (molecule.empty()){ return false;}
    std::string head = molecule[0];
    return ops.count(head) & !this->isbimol(molecule);
}

float fraglets::propensity(){
    this->run_unimol();
    this->prop.clear();
    this->wt = 0;
    keyMultisetMap::iterator it = this->active.keyMap.begin();
    for (;it != this->active.keyMap.end();it++){
        std::string key = it->first;
        moleculeMultiset mset = *it->second;
        std::size_t m = mset.multiset.size();
        moleculeMultiset passive_mset = *this->passive.keyMap[key];
        std::size_t p = passive_mset.multiset.size();
        std::size_t w = m*p;
        if (w > 0){
            this->prop[key] = w;
        }
        this->wt += w;
    }
    if (this->wt <= 0){this->idle = true;}
    return this->wt;

}

void fraglets::react(float w){
        // """ perform the selected reaction pointed to by the dice position w
        //     (typically involked from the hierarchical Gillespie SSA
        // """
        if (this->wt < 0){ return;}
        keyMultisetMap::iterator it = this->active.keyMap.begin();
        for (;it != this->active.keyMap.end();it++){
            std::string key = it->first;
            propMapIterator it = this->prop.find(key);
            if(it != this->prop.end()){
                float propValue = it->second;
                if (propValue > 0 and w < propValue  ){
                    int r = rand() % this->active.keyMap.size();
                    // molecule mol1 = this->active.

                }
            }


        }

// map<int,Bar>::iterator it = m.find('2');
// Bar b3;
// if(it != m.end())
// {
//    //element found;
//    b3 = it->second;
// }


}

//    def react(self, w):
//         """ perform the selected reaction pointed to by the dice position w
//             (typically involked from the hierarchical Gillespie SSA
//             implementation in Cell.py)
//         """
//         if self.wt <= 0: return
//         for k in self.active.keys():
//             if (k in self.prop):
//                 if self.prop[k] > 0 and w < self.prop[k]:
//                     mol1 = self.active.expelrnd(k)
//                     mol2 = self.passive.expelrnd(k)
//                     res = self.react2(mol1, mol2)
//                     self.inject_list(res)
//                     return
//                 w -= self.prop[k]



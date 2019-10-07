#include "fraglets.h"

void fraglets::inject(molecule molecule,int mult=1){
    if (molecule.empty() | mult < 1){return;}
    if (molecule.size() >1){
        if (this->isbimol(molecule)){
                std::string key = molecule[1];
                // could check for invalid fraglets here.
                if (this->active.count(key) > 0){
                    molecule_multiset mset;
                    this->active[key] = &mset;
                }
                molecule_multiset mset = *this->active[key];
                for (int i =0; i<mult;i++){
                    mset.insert(&molecule);
                }
                this->idle = false;
            }
        else if (this->isunimol(molecule)){
            for (int i = 0; i<mult;i++){
                this->unimol.insert(&molecule);
            }
        }
        else{
            std::string key = molecule[1];
            // could check for invalid fraglets here.
            if (this->passive.count(key) > 0){
                molecule_multiset mset;
                this->passive[key] = &mset;
            }
            
            molecule_multiset mset = *this->passive[key];
            for (int i =0; i<mult;i++){
                mset.insert(&molecule);
            }
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
    keyMultisetIterator it = this->active.begin();
    for (;it != this->active.end();it++){
        std::string key = it->first;
        molecule_multiset mset = *it->second;
        std::size_t m = mset.size();
        molecule_multiset passive_mset = *this->passive[key];
        std::size_t p = passive_mset.size();
        std::size_t w = m*p;
        if (w > 0){
            this->prop[key] = w;
        }
        this->wt += w;
    }
    if (this->wt <= 0){this->idle = true;}
    return this->wt;

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

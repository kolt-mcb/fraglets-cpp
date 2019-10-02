#include "fraglets.h"
using namespace std;

void fraglets::inject(vector<string> molecule,int mult=1){
    if (molecule.empty() | mult < 1){return;}
    if (this->isbimol(molecule)){
        if (molecule.size() >1){
            string key = molecule[1];
            // could check for invalid fraglets here.
            if (this->active.count(key) > 0){
                molecule_multiset mset;
                this->active[key] = &mset;
            }
   
            for (int i =0; i<mult;i++){
                molecule_multiset mset;
                mset.insert(&molecule);
            }
            this->idle = false;
        }
    else if (this->isunimol(molecule)){

    }
//             if len(mol) > 1:
//                 self.unimol.inject(mol, mult)
//                 self.idle = False
//             # else discard invalid fraglet
    else{return;}
    }
}   
bool fraglets::isbimol(vector<string> molecule){
    if (molecule.empty()){ return false;}
    string head = molecule[0];
    return head == "match" | head == "matchp";
}
bool fraglets::isunimol(vector<string> molecule){
    if (molecule.empty()){ return false;}
    string head = molecule[0];
    return ops.count(head) & !this->isbimol(molecule);
}

    // def isbimol(self, mol):
    //     """ true if fraglet 'mol' starts with a bimolecular reaction rule """
    //     if mol == '': return False
    //     return mol[0] == self.instr['match'] or mol[0] == self.instr['matchp']
    // def inject( self, key, mol, mult=1 ):
    //     """ inject a given amount of a molecule in the multiset,
    //         indexed by the provided key
    //     """
	// if (key == '' or mol == '' or mult < 1): return
	// if (key in self.keymset):
    //         self.keymset[key].inject(mol, mult)
	// else:
    //         self.keymset[key] = Multiset()
    //         self.keymset[key].inject(mol, mult)
	// self.total += mult





//  def inject( self, mol, mult=1 ):
//         """ inject 'mult' copies of fraglet 'mol' in the reactor """
// 	if (mol == '' or mult < 1): return
//         if (self.isbimol(mol)):
//             if len(mol) > 1:
//                 key = mol[1]
//                 #if key in self.op: # invalid fraglet
//                 #    while len(mol) > 1 and self.isbimol(mol):
//                 #        # eliminate [match match match ...] sequences
//                 #        mol = mol[1:]
//                 #    #pending: clean fraglet
//                 self.active.inject(key, mol, mult)
//                 self.idle = False
//             # else discard invalid fraglet
//         elif (self.isunimol(mol)):
//             if len(mol) > 1:
//                 self.unimol.inject(mol, mult)
//                 self.idle = False
//             # else discard invalid fraglet
//         else:
//             key = mol[0]
//             self.passive.inject(key, mol, mult)
//             self.idle = False
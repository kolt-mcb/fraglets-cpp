
#include <algorithm>
#include <iostream>
#include <chrono>
#include <random>
#include "fraglets.h"



std::string match  = "match" ;
std::string matchp =  "matchp";
std::string dup    = "dup";
std::string exch   = "exch";
std::string pop    = "pop";
std::string nop    = "nop";
std::string nul    = "nul";
std::string split  = "split";
std::string send   = "send";
std::string fork   = "fork";

std::unordered_set<std::string> bimolTags = {match,matchp};
std::unordered_set<std::string> unimolTags = {dup,exch,pop,nop,nul,split,send,fork};




opResult r_match(molecule activeMolecule, molecule passiveMolecule){
    opResult result;

    molecule newMol;
    newMol = molecule();
    newMol.insert(newMol.end(),activeMolecule.begin() + 2, passiveMolecule.end());
    newMol.insert(newMol.end(),passiveMolecule.begin()+1,passiveMolecule.end());
    result[0] = newMol;
    return result;
}


opResult r_matchp(molecule activeMolecule, molecule passiveMolecule){
    opResult result =  r_match(activeMolecule,passiveMolecule);
    // opResult::iterator it = result.begin();
    // molecule::iterator ait = activeMolecule.begin();
    // molecule::iterator eit = activeMolecule.end();

    // result.insert(it,ait,eit);
    opResult newResult;
    newResult[0] = activeMolecule;
    newResult[1] = result[0];
    return newResult;
}

opResult r_fork(molecule mol){
    opResult result;
    if (mol.size()<2) {return result;}
    if (mol.size()<3) {
        molecule newMol(mol.begin()+1,mol.end());
        result[0] = newMol;
        return result;
    } 
    molecule mol1;
    molecule mol2;
    if (mol.size() < 4){
        mol1[0] = mol[1];
        mol2[0] = mol[2];
    }
    else{
        mol1.insert(mol1.begin(),mol.begin()+1,mol.begin()+1);
        mol1.insert(mol1.end(),mol.begin()+3,mol.end());
        mol2.insert(mol2.begin(),mol.begin()+2,mol.end());
        result.insert(result.begin(),mol1);
        result.insert(result.end(),mol2);
    }
    
    return result;
}
opResult r_dup(molecule mol){
    opResult result;
    return result;
}


opResult r_nop(molecule mol){
    opResult result;
    if (mol.size() < 2){
        return result;
    }
    molecule newMol = molecule(mol.begin()+1,mol.end());
    result[0] = newMol;
    return result;
}
opResult r_nul(molecule mo){
    opResult result;
    return result;
}
opResult r_split(molecule mo){
    opResult result;
    return result;
}
opResult r_exch(molecule mo){
    opResult result;
    return result;
}
opResult r_send(molecule mo){
    opResult result;
    return result;
}
opResult r_pop(molecule mo){
    opResult result;
    return result;
}



std::unordered_map<std::string,bimolOp> bimolMap = {{match,r_match},{matchp,r_matchp}};
std::unordered_map<std::string,unimolOp> unimolMap = {{dup,r_dup},
                                                      {exch,r_exch},
                                                      {pop,r_pop},
                                                      {nop,r_nop},
                                                      {nul,r_nul},
                                                      {split,r_split},
                                                      {send,r_send},
                                                      {fork,r_fork}
                                                    };



double random_double(){
    std::mt19937_64 rng;
    // initialize the random number generator with time-dependent seed
    uint64_t timeSeed = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    std::seed_seq ss{uint32_t(timeSeed & 0xffffffff), uint32_t(timeSeed>>32)};
    rng.seed(ss);
    // initialize a uniform distribution between 0 and 1
    std::uniform_real_distribution<double> unif(0, 1);
    return unif(rng);
}

void fraglets::inject(molecule molecule,int mult){

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
    return head == match | head == matchp;
}
bool fraglets::isunimol(molecule molecule){
    if (molecule.empty()){ return false;}
    std::string head = molecule[0];
    return unimolTags.find(head) != unimolTags.end() & !this->isbimol(molecule);
}

double fraglets::propensity(){
    this->run_unimol();
    this->prop.clear();
    this->wt = 0;
    keyMultisetMap::iterator it = this->active.keyMap.begin();
    std::cout << "propensity" << this->active.keyMap.size();
    for (;it != this->active.keyMap.end();it++){
        std::string key = it->first;
        moleculeMultiset mset = *it->second;
        std::size_t m = mset.multiset.size();
        moleculeMultiset passive_mset = *this->passive.keyMap[key];
        std::size_t p = passive_mset.multiset.size();
        std::size_t w = m*p;
        std::cout << "test " << w;
        if (w > 0){
            this->prop[key] = w;
        }
        this->wt += w;
    }
    if (this->wt <= 0){this->idle = true;}
    return this->wt;

}

void fraglets::react(double w){
        // """ perform the selected reaction pointed to by the dice position w
        //     (typically involked from the hierarchical Gillespie SSA
        // """
        if (this->wt < 0){ return;}
        keyMultisetMap::iterator it = this->active.keyMap.begin();
        for (;it != this->active.keyMap.end();it++){
            std::string key = it->first;
            propMapIterator it = this->prop.find(key);
            if(it != this->prop.end()){
                double propValue = it->second;
                if (propValue > 0 and w < propValue  ){
                    molecule activeMolecule = this->active.expelrnd(key);
                    molecule passiveMolecule = this->passive.expelrnd(key);
                    std::vector<molecule> result = this->react2(activeMolecule,passiveMolecule);
            }
        }
    }
}

opResult fraglets::react1(molecule mol){
    std::string tag = mol[0];
    opResult result;
    unimolOp f = unimolMap.find(tag)->second;
    result = f(mol);
    return result;
}

opResult fraglets::react2(molecule activeMolecule,molecule passiveMolecule ){
    std::string tag = activeMolecule[0];
    opResult result;
    bimolOp f = bimolMap.find(tag)->second;
    result = f(activeMolecule,passiveMolecule);
    return result;
}

int fraglets::run_unimol(){
    int n = 0;

    while (!this->unimol.multiset.empty()){
        std::cout << this->unimol.multiset.size();
        molecule mol = this->unimol.expelrnd();
        opResult result = this->react1(mol);
        this->inject_list(result);
        n++;
    }
    return n;
}


void fraglets::run_bimol(){
    std::cout << "test";
    std::cout << this->wt;
    if (this->wt <= 0){return;}
    double w = random_double() * this->wt;
    std::cout << w;
    this->react(w);
}


void fraglets::inject_list(opResult result){
    opResult::iterator it = result.begin();
    for (;it!=result.end();it++){
        molecule mol = *it;
        this->inject(mol);
    }
}


void fraglets::iterate(){
    this->propensity();
    if (!this->idle){
        this->run_bimol();
    }

}
void fraglets::run(int niter){
    for (int i = 0;i<niter;i++){
        std::cout << "ITER=" << i;
        this->iterate();
        if (this->idle){return;}
    }
}



// opResult r_match(molecule,molecule);
// opResult r_matchp(molecule,molecule);
// opResult r_fork(molecule);
// opResult r_dup(molecule);
// opResult r_nop(molecule);
// opResult r_nul(molecule);
// opResult r_split(molecule);
// opResult r_exch(molecule);
// opResult r_send(molecule);
// opResult r_pop(molecule);


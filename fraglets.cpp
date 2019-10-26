#include "fraglets.h"
#include <chrono>
#include <sstream>
#include <iterator>
#include <cstddef> 


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


std::vector<std::string> moleculeToVector(std::string mol){
    std::istringstream iss(mol);
    std::vector<std::string> result{
        std::istream_iterator<std::string>(iss), {}
    };
    return result;
}

molecule vectorToString(std::vector<std::string> vec){
    std::ostringstream vts; 
    std::copy(vec.begin(), vec.end()-1, 
    std::ostream_iterator<std::string>(vts, " ")); 
  
    // Now add the last element with no delimiter 
    vts << vec.back(); 
    return vts.str();
}



opResult r_match(molecule activeMolecule, molecule passiveMolecule){
    opResult result;
    
    size_t postMatch = activeMolecule.find_first_of(" ");
    size_t postPassive = activeMolecule.find_first_of(" ",postMatch+1);
    molecule* newMol = new molecule(activeMolecule.substr(postPassive+1,activeMolecule.size()-(postPassive-1)));
    size_t passivePostPassive = passiveMolecule.find_first_of(" ");
    newMol->append(passiveMolecule.substr(passivePostPassive,passiveMolecule.size()-passivePostPassive));
    result.insert(result.begin(),*newMol);
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
    int total = 0;
    for(int i = 0;i<mol.size() & total < 5;i++){
        if (mol[i] == *" "){
            total ++;
        }
    }
    if (total<2) {return result;}
    if (total<3) {
        molecule newMol;
        size_t found = mol.find_first_of(" ");
        newMol = mol.substr(found+1,mol.back()-(found-1));
        result[0] = newMol;
        return result;
    } 
    molecule mol1;
    molecule mol2;
    if (total < 4){
        size_t postFork = mol.find_first_of(" ");
        size_t postMol1 = mol.find_first_of(" ",postFork);
        mol1 = mol.substr(postFork+1,mol.back()-(postFork+1));;
        mol2 = mol.substr(postMol1+1,mol.back()-(postMol1+1));
    }
    else{
        size_t postFork = mol.find_first_of(" ");
        size_t postMol1 = mol.find_first_of(" ",postFork+1);
        size_t postMol2 = mol.find_first_of(" ",postMol1+1);
        molecule mol1mol = mol.substr(postFork+1,postMol1-(postFork+1));
        molecule mol2mol = mol.substr(postMol1+1,postMol2-(postMol1+1));
        mol1.append(mol1mol);
        mol2.append(mol2mol);
        molecule tail = mol.substr(postMol2,mol.back()-postMol2);

        mol1.append(tail);
        mol2.append(tail);

    }


    result.push_back(mol1);
    result.push_back(mol2);
    return result;
}
opResult r_dup(molecule mol){
    opResult result;
    return result;
}


opResult r_nop(molecule mol){
    opResult result;

    size_t found = mol.find(" ");
    if (found!=mol.back()){
        molecule newMol = mol.substr(found+1,mol.back());
        result.insert(result.end(),newMol);
    }
    return result;
}

opResult r_nul(molecule mo){
    opResult result;
    return result;
}
opResult r_split(molecule mol){
    opResult result;
    int total = 0;
    for(int i = 0;i<mol.size() & total < 3;i++){
        if (mol[i] == *" "){
            total ++;
        }
    }
    if (total < 2){return result;}

    molecule mol1;
    size_t postExch = mol.find_first_of(" ");
    size_t found = mol.find_first_of('*');
    if (found != std::string::npos){
        molecule mol2;
        mol1.append(mol.substr(postExch+1,found-postExch));
        mol2.append(mol.substr(found+1,mol.size()-(found+1)));
        result.push_back(mol1);
        result.push_back(mol2);
    }else
    {
        mol1.append(mol.substr(postExch+1,mol.size()-(postExch+1)));
        result.push_back(mol1);
    }
    return result;
}

        // if len(mol) < 2: return ''
        // return mol[1:].split('*', 1)


opResult r_exch(molecule mol){
    opResult result;

    int total = 0;
    for(int i = 0;i<mol.size() & total < 6;i++){
        if (mol[i] == *" "){
            total ++;
        }
    }
    if (total < 2){
        return result;
    }
    else if (total < 4){
        size_t postExch = mol.find_first_of(" ");
        molecule newMol = molecule(mol.substr(postExch,mol.size()-postExch));
        result.push_back(newMol);
        return result;
    }
    else if (total < 5){
        molecule newMol;
        size_t postExch = mol.find_first_of(" ");
        size_t postMol1 = mol.find_first_of(" ",postExch+1);
        size_t postMol2 = mol.find_first_of(" ",postMol1+1);
        size_t postMol3 = mol.find_first_of(" ",postMol2+1);
        newMol.append(mol.substr(postExch+1,postMol1-(postExch+1)));
        newMol.append(mol.substr(postMol2+1,postMol3-(postMol2+1)));
        newMol.append(mol.substr(postMol1+1,postMol2-(postMol1+1)));
        result.push_back(newMol);
        return result;
    }


    molecule newMol;
    size_t postExch = mol.find_first_of(" ");
    size_t postMol1 = mol.find_first_of(" ",postExch+1);
    size_t postMol2 = mol.find_first_of(" ",postMol1+1);
    size_t postMol3 = mol.find_first_of(" ",postMol2+1);
    newMol.append(mol.substr(postExch+1,postMol1-(postExch+1)));
    newMol.append(mol.substr(postMol2+1,postMol3-(postMol2+1)));
    newMol.append(mol.substr(postMol1+1,postMol2-(postMol1+1)));
    newMol.append(mol.substr(postMol3+1,mol.size()-(postMol3+1)));
    result.push_back(newMol);
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



std::unordered_map<std::string,bimolOp>  const bimolMap = {{match,r_match},{matchp,r_matchp}};
std::unordered_map<std::string,unimolOp> const unimolMap = {{dup,r_dup},
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

void fraglets::inject(molecule mole,int mult){
    moleculeVector moleVector = moleculeToVector(mole);
    if (moleVector.empty() | mult < 1){return;}
    if (moleVector.size() >1){
        if (this->isbimol(mole)){   
                std::string key = moleVector[1];
                // could check for invalid fraglets here.
                this->active.inject(key,mole,mult);
                this->idle = false;
            }
        else if (this->isunimol(mole)){
            for (int i = 0; i<mult;i++){
                this->unimol.inject(mole,mult);
            }
        }
        else{
            std::string key = moleVector[0];
            // could check for invalid fraglets here.
            this->passive.inject(key,mole,mult);
            this->idle = false;
        }               
    }
}   
bool fraglets::isbimol(molecule mol){
    if (mol.empty()){ return false;}
    
    size_t  postTag = mol.find_first_of(" ");
    if (postTag != std::string::npos){
        molecule tag = mol.substr(0,postTag);
        return tag == match | tag == matchp;
    }

    return false;
}
bool fraglets::isunimol(molecule mole){
    if (mole.empty()){ return false;}
    size_t found;
    found = mole.find(" ");
    molecule head = mole.substr(0,found);
    return unimolTags.find(head) != unimolTags.end() & !this->isbimol(mole);
}

double fraglets::propensity(){
    this->run_unimol();
    this->prop.clear();
    this->wt = 0;
    keyMultisetMap::iterator it = this->active.keyMap.begin();
    for (;it != this->active.keyMap.end();it++){
        std::string key = it->first;
        moleculeMultiset mset = *it->second;
        std::size_t m = mset.multiset.size();
        moleculeMultiset passive_mset = *this->passive.keyMap.find(key)->second;
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
                    opResult result = this->react2(activeMolecule,passiveMolecule);
                    this->inject_list(result);
                    return;
            }
            w -= this->prop[key];
        }
    }
}


opResult fraglets::react1(molecule mol){
    size_t found = mol.find(" ");

    std::string tag = mol.substr(0,found);
    opResult result;
    unimolOp f = unimolMap.find(tag)->second;
    result = f(mol);
    return result;
}

opResult fraglets::react2(molecule activeMolecule,molecule passiveMolecule ){
    size_t found = activeMolecule.find(" ");

    std::string tag = activeMolecule.substr(0,found);
    opResult result;
    bimolOp f = bimolMap.find(tag)->second;
    result = f(activeMolecule,passiveMolecule);

    std::cout << "[" <<activeMolecule << "] + [" << passiveMolecule << "] -> [" << result[0] << "]\n";
    return result;
}

int fraglets::run_unimol(){
    int n = 0;
    // std::cout << "wtf\n" <<(!this->unimol.multiset.empty());
    while (!this->unimol.multiset.empty()){
        molecule mol = this->unimol.expelrnd();
        opResult result = this->react1(mol);
        if (result.size() == 1){
            std::cout << "[" << mol << "] -> [" << result[0] << "]\n" ;
        }   
        if (result.size() == 2){
            std::cout << "[" << mol << "] -> [" << result[0] << "] & [" << result[1] << "]\n" ;
        }             
        this->inject_list(result);
        n++;
    }
    return n;
}


void fraglets::run_bimol(){
    if (this->wt <= 0){return;}
    double w = random_double() * this->wt;
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
        std::cout<< "ITER="<<i<<'\n';
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


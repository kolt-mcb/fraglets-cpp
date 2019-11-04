#include "fraglets.h"
#include <chrono>
#include <sstream>
#include <iterator>
#include <cstddef> 
#include <fstream>

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
std::string empty  = "empty";
std::string length = "length";
std::string lt     = "lt";

bool isNumber(const molecule& mol){
    try
    {
        std::stoi(mol);
        return true;
    }
    catch(const std::exception& e)
    {
        return false;
    }
    
}


std::unordered_set<std::string> bimolTags = {match,matchp};
std::unordered_set<std::string> unimolTags = {dup,exch,pop,nop,nul,split,send,fork,empty,length,lt};


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

    size_t singlet = passiveMolecule.find_first_of(" ");
    size_t postMatch = activeMolecule.find_first_of(" ");
    size_t postPassive = activeMolecule.find_first_of(" ",postMatch+1);
    molecule newMol = molecule(activeMolecule.substr(postPassive+1,activeMolecule.size()-(postPassive-1)));
    if (singlet != std::string::npos){
        newMol.append(passiveMolecule.substr(singlet,passiveMolecule.size()-singlet));
    }
    result.insert(result.begin(),newMol);
    return result;
}


opResult r_matchp(molecule activeMolecule, molecule passiveMolecule){
    opResult result =  r_match(activeMolecule,passiveMolecule);
    // opResult::iterator it = result.begin();
    // molecule::iterator ait = activeMolecule.begin();
    // molecule::iterator eit = activeMolecule.end();

    // result.insert(it,ait,eit);
    opResult newResult;
    newResult.push_back(activeMolecule);
    newResult.push_back(result[0]);
    return newResult;
}

opResult r_fork(molecule mol){
    opResult result;
    int total = 0;
    for(int i = 0;(i<mol.size()) and (total < 5);i++){
        if (mol[i] == *" "){
            total ++;
        }
    }
    if (total<2) {return result;}
    if (total<3) {
        molecule newMol;
        size_t found = mol.find_first_of(" ");
        newMol = mol.substr(found+1,mol.back()-(found-1));
        result.push_back(newMol);
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
        molecule tail = mol.substr(postMol2,mol.size());

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
        molecule newMol = mol.substr(found+1,mol.size());
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
    for(int i = 0;(i<mol.size()) & (total < 3);i++){
        if (mol[i] == *" "){
            total ++;
        }
    }
    if (total < 2){return result;}

    molecule mol1;
    size_t postSplit = mol.find_first_of(" ");
    size_t found = mol.find_first_of('*');
    if (found != std::string::npos){
        molecule mol2;
        mol1.append(mol.substr(postSplit+1,found-(postSplit+2)));
        mol2.append(mol.substr(found+2,mol.size()-(found+1)));
        result.push_back(mol1);
        result.push_back(mol2);
    }else
    {
        mol1.append(mol.substr(postSplit+1,mol.size()-(postSplit+1)));
        result.push_back(mol1);
    }
    return result;
}


opResult r_exch(molecule mol){
    opResult result;

    int total = 0;
    for(int i = 0;(i<mol.size()) & (total < 6);i++){
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
    newMol.append(mol.substr(postMol2,postMol3-(postMol2)));
    newMol.append(mol.substr(postMol1,postMol2-(postMol1)));
    newMol.append(mol.substr(postMol3,mol.size()-(postMol3)));
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

opResult r_empty(molecule mol){
    opResult result;

    int total = 0;
    for(int i = 0;(i<mol.size()) & (total < 4);i++){
        if (mol[i] == *" "){
            total ++;
        }
    }

    size_t postEmpty = mol.find_first_of(" ");
    size_t postMol1 = mol.find_first_of(" ",postEmpty+1);
    size_t postMol2 = mol.find_first_of(" ",postMol1+1);
    size_t postMol3 = mol.find_first_of(" ",postMol2+1);
    if (total < 3){
        return result;
    }
    else if (total == 3){
        molecule newMol = mol.substr(postEmpty+1,postMol1-1);
        result.push_back(newMol);
    }
    else if (total > 3){
        molecule newMol = mol.substr(postMol1+1,mol.size()-(postMol1+1));
        result.push_back(newMol);
    }
    return result;
}


opResult r_length(molecule mol){
    opResult result;
    int total = 0;
    for(int i = 0;i<mol.size();i++){
        if (mol[i] == *" "){
            total ++;
        }
    }

    if (total < 3){
        return result;
    }
    else{
        size_t postLength = mol.find_first_of(" ");
        size_t postMol1 = mol.find_first_of(" ",postLength+1);
        size_t postMol2 = mol.find_first_of(" ",postMol1+1);
        size_t postMol3 = mol.find_first_of(" ",postMol2+1);
        molecule newMol = mol.substr(postLength+1,postMol1-postLength);
        newMol.append(std::to_string(total));
        newMol.append(mol.substr(postMol1,mol.size()-(postMol1)));
        result.push_back(newMol);
        return result;
        
    }
}


opResult r_lessthan(molecule mol){
    opResult result;
    molecule newMol;


    int total = 0;
    for(int i = 0;(i<mol.size()) or (i<3);i++){
        if (mol[i] == *" "){
            total ++;
        }
    }
    if (total > 3){
        size_t postLt = mol.find_first_of(" ");
        size_t postMol1 = mol.find_first_of(" ",postLt+1);
        size_t postMol2 = mol.find_first_of(" ",postMol1+1);
        size_t postMol3 = mol.find_first_of(" ",postMol2+1);
        size_t postMol4 = mol.find_first_of(" ",postMol3+1);
        molecule m1 = mol.substr(postMol2+1,postMol3-1);
        molecule m2 = mol.substr(postMol3+1,postMol4-1);
        if (isNumber(m1) and isNumber(m2)){
            float n1  = std::stoi(m1);
            float n2 = std::stoi(m2);
            if (n1 < n2){
                newMol = mol.substr(postLt+1,postMol1-1);
            }else{
                newMol = mol.substr(postMol1+1,postMol2-1);
            }
            newMol.append(mol.substr(postMol2+1,mol.size()-(postMol2+1)));
            result.push_back(newMol);
        } 
        
    }
    return result;
    
}
    // def r_lessThan(self,mol):
    //     """[lt yes no n m tail]  --> if n<m: [yes n m tail]
    //                                  else:   [no n m tail]"""
    //     newMol = tuple()
    //     if len(mol) > 4:
    //         if  is_number_tryexcept(mol[3]) and  is_number_tryexcept(mol[4]):
    //             if float(mol[3]) < float(mol[4]):
    //                 newMol = newMol + (mol[1],)
    //             else:
    //                 newMol = newMol + (mol[2],)
    //             newMol = newMol + (mol[3:])
    //     return newMol

std::unordered_map<std::string,bimolOp>  const bimolMap = {{match,r_match},{matchp,r_matchp}};
std::unordered_map<std::string,unimolOp> const unimolMap = {{dup,r_dup},
                                                      {exch,r_exch},
                                                      {pop,r_pop},
                                                      {nop,r_nop},
                                                      {nul,r_nul},
                                                      {split,r_split},
                                                      {send,r_send},
                                                      {fork,r_fork},
                                                      {empty,r_empty},
                                                      {length,r_length},
                                                      {lt,r_lessthan}
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




void fraglets::inject(const molecule& mol,int mult){
    if (mol.empty() or mult < 1){return;}
    int total = 0;
    for(int i = 0;(i<mol.size()) & (total < 2);i++){
        if (mol[i] == *" "){
            total ++;
        }
    }

    if (total>= 1){

        size_t postTag = mol.find_first_of(" ");
        size_t postMol1 = mol.find_first_of(" ",postTag+1);
        if (this->isbimol(mol)){   
                std::string key = mol.substr(postTag+1,postMol1-(postTag+1));
                // could check for invalid fraglets here.
                this->active.inject(key,mol,mult);
                this->idle = false;
            }
        else if (this->isunimol(mol)){
            this->unimol.inject(mol,mult);
            }
        else{
            std::string key = mol.substr(0,postTag);
            // could check for invalid fraglets here.
            this->passive.inject(key,mol,mult);
            this->idle = false;
        }                  
    }
    else{
        this->passive.inject(mol,mol,mult);
        this->idle = false;             
    }
}

bool fraglets::isbimol(const molecule& mol){
    if (mol.empty()){ return false;}
    size_t  postTag = mol.find_first_of(" ");
    if (postTag != std::string::npos){
        molecule tag = mol.substr(0,postTag);
        return (tag == match) or (tag == matchp);
    }

    return false;
}
bool fraglets::isunimol(const molecule& mol){
    if (mol.empty()){ return false;}
    size_t found;
    found = mol.find(" ");
    molecule head = mol.substr(0,found);
    return (unimolTags.find(head) != unimolTags.end()) & !this->isbimol(mol);
}

double fraglets::propensity(){
    this->run_unimol();

    this->prop.clear();
    this->wt = 0;
    keyMultisetMap::iterator it = this->active.keyMap.begin();
    for (;it != this->active.keyMap.end();it++){
        std::string key = it->first;
        std::size_t m = this->active.multk(key);
        std::size_t p = this->passive.multk(key);
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
            propMapIterator pit = this->prop.find(key);
            if(pit != this->prop.end()){
                double propValue = pit->second;
                if ((propValue > 0) and (w < propValue)){
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


opResult fraglets::react1(const molecule& mol){
    size_t found = mol.find(" ");

    std::string tag = mol.substr(0,found);
    opResult result;
    unimolOp f = unimolMap.find(tag)->second;
    result = f(mol);
    return result;
}

opResult fraglets::react2(const molecule& activeMolecule,const molecule& passiveMolecule ){
    size_t found = activeMolecule.find(" ");

    std::string tag = activeMolecule.substr(0,found);
    opResult result;
    bimolOp f = bimolMap.find(tag)->second;
    result = f(activeMolecule,passiveMolecule);
    if (result.size() == 1){
        std::cout << "[" <<activeMolecule << "] , [" << passiveMolecule << "] -> \n[" << result[0] << "]\n";
    }
    if (result.size() == 2){
        std::cout << "[" <<activeMolecule << "] , [" << passiveMolecule << "] -> \n[" << result[0] << "] , [" << result[1] << "]\n" ;
    }
    return result;
}

int fraglets::run_unimol(){
    int n = 0;
    while (!this->unimol.multiset.empty()){
        molecule mol = this->unimol.expelrnd();
        opResult result = this->react1(mol);
        if (result.size() == 1){
            std::cout << "[" << mol << "] ->\n[" << result[0] << "]\n" ;
        }   
        if (result.size() == 2){
            std::cout << "[" << mol << "] ->\n[" << result[0] << "] , [" << result[1] << "]\n" ;
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
    this->activeMultisetSize.push_back(this->active.total);
    this->passiveMultisetSize.push_back(this->passive.total);

}
void fraglets::run(int niter){
    for (int i = 1;i<niter;i++){
        this->trace();
        std::cout<< "ITER="<<i<<'\n' ;
        this->iterate();
        if (this->idle){return;}
    }
}


void fraglets::parse(std::string line){

    
    if ((line[0] == '#') or line.empty()){
        return;
    }

    // if (bracket2 != line.back()){

    // } 

    size_t bracket1 = line.find_first_of("[");
    size_t bracket2 = line.find_first_of("]");
    molecule mol = molecule(line.substr(bracket1+1,(line.size()-bracket1)-2));
    this->inject(mol);


}

void fraglets::interpret(std::string filename){
    // attach an input stream to the wanted file
    std::ifstream input_stream(filename);

    // check stream status
    if (!input_stream) std::cerr << "Can't open input file!";


    // one line
    std::string line;

    // extract all the text from the input file
    while (getline(input_stream, line)) {

        // store each line in the vector
        this->parse(line);
    }


}

void fraglets::trace(){
    std::cout << "================================\n";
    keyMultisetMap::iterator ait = this->active.keyMap.begin();
    for (;ait!=this->active.keyMap.end();ait++){
        moleculeMultiset amset = *ait->second;
        unorderedMultiset::iterator amit = amset.multiset.begin();
        for(;amit!=amset.multiset.end();amit++){
            std::cout << *amit << '\n';
        }
    }
    keyMultisetMap::iterator pit = this->passive.keyMap.begin();
    for (;pit!=this->passive.keyMap.end();pit++){
        moleculeMultiset pmset = *pit->second;
        unorderedMultiset::iterator pmit = pmset.multiset.begin();
        for(;pmit!=pmset.multiset.end();pmit++){
            std::cout << *pmit << '\n';
        }
    }
    std::cout << "================================\n";
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


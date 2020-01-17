#include "fraglets.h"
#include <chrono>
#include <sstream>
#include <iterator>
#include <cstddef>
#include <fstream>
#include <string>
#include <iterator>
#include <cstring>



symbol match  = "match" ;
symbol matchp =  "matchp";
symbol dup    = "dup";
symbol exch   = "exch";
symbol pop    = "pop";
symbol nop    = "nop";
symbol nul    = "nul";
symbol split  = "split";
symbol send   = "send";
symbol fork   = "fork";
symbol empty  = "empty";
symbol length = "length";
symbol lt     = "lt";
symbol pop2   = "pop2";
symbol copy   = "copy";
symbol perm   = "perm";

std::unordered_set<std::string> bimolTags = {match,matchp,perm};
std::unordered_set<std::string> unimolTags = {exch,pop,nop,split,length,fork,empty,pop2,copy,lt};



bool isNumber(const std::string& mol){
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

symbol molToString(const molecule_pointer mol){
    const char* const delim = " ";
    std::string nodeString;

    for (auto sym : mol->vector){
        nodeString.append(*sym);
        nodeString.push_back(' ');
    }

    return nodeString;


}




const molecule_pointer fraglets::makeUniqueUnimol(const molecule_pointer mol){
    moleculeMap::iterator it = this->unimolMap.find(*mol);
    if (it != this->unimolMap.end()){
        return it->second;
    }else{
        this->unimolMap.insert(std::pair(*mol,mol));
    }
    return mol;
}

const molecule_pointer fraglets::makeUniqueActive(const molecule_pointer mol){
    moleculeMap::iterator it = this->activeMap.find(*mol);
    if (it != this->activeMap.end()){
        return it->second;
    }else{
        this->activeMap.insert(std::pair(*mol,mol));
    }
    return mol;
}

const molecule_pointer fraglets::makeUniquePassive(const molecule_pointer mol){
    moleculeMap::iterator it = this->passiveMap.find(*mol);
    if (it != this->passiveMap.end()){
        return it->second;
    }else{
        this->passiveMap.insert(std::pair(*mol,mol));
    }
    return mol;
}


void fraglets::addNode(const molecule_pointer mol,const bool& unimol,const bool& matchp,const bool& bimol){
    symbol _mol = molToString(mol);
    char* c_mol = &_mol[0];//new char[_mol.size() + 1];

    //_mol.copy(c_mol,_mol.size(),_mol.front());
    // std::transform(mol->vector.begin(),mol->vector.end(),std::back_inserter(c_mol),convert);

    //c_mol[mol->vector.size()] = '\0';
    Agnode_t* node;
    if (matchp){
        node =  agnode(this->subgraph,c_mol,TRUE);
        agsafeset(node,"shape","hexagon","hexagon");
    }else{
        node =  agnode(this->graph,c_mol,TRUE);
        agsafeset(node,"shape","circle","circle");
    }
    if(unimol){
        agsafeset(node,"color","blue","blue");
    }
    if(bimol){
        agsafeset(node,"color","red","red");
    }
    agsafeset(node,"penwidth","10","10");
    this->nodesTable[mol] = node;
}

void fraglets::addEdge(const molecule_pointer mol,const molecule_pointer resultMol,const bool& unimol,const bool& matchp){

    // std::string molString = molToString(mol);

    // std::string resultMolString =  molToString(resultMol);
  

    if (this->nodesTable.find(mol) == this->nodesTable.end()){
        this->addNode(mol,isunimol(mol),isperm(mol),isbimol(mol));
    }
    if (this->nodesTable.find(resultMol) == this->nodesTable.end()){
        this->addNode(resultMol,isunimol(resultMol),isperm(resultMol),isbimol(resultMol));
    }
    Agnode_t* tailNode = nodesTable[resultMol];
    Agnode_t* headNode = nodesTable[mol];
    Agedge_t* edge = agedge(this->graph,headNode,tailNode,"",true);
    if (unimol){
        agsafeset(edge,"color","blue","blue");
    }else if (matchp){
        agsafeset(edge,"color","green","green");
        agsafeset(edge,"dir","none","none");
    }else{
        agsafeset(edge,"color","black","black");
    }
    this->edgeTable[mol] = edge;
}
opResult r_match(const molecule_pointer activeMolecule, const molecule_pointer passiveMolecule){
    opResult result;


    molecule_pointer newMol = std::make_shared<molecule>();
    newMol->vector.insert(newMol->vector.begin(),activeMolecule->vector.begin()+2,activeMolecule->vector.end());
    newMol->vector.insert(newMol->vector.end(),passiveMolecule->vector.begin()+1,passiveMolecule->vector.end());
    result.insert(result.begin(),newMol);
    return result;
}


opResult r_perm(const molecule_pointer activeMolecule, const molecule_pointer passiveMolecule){
    opResult result =  r_match(activeMolecule,passiveMolecule);
    // molecule newMol = std::make_shared<molecule_pointer( activeMolecule);
    result.emplace(result.begin(),activeMolecule);
    return result;
}

opResult r_matchp(const molecule_pointer activeMolecule, const molecule_pointer passiveMolecule){
    opResult result =  r_match(activeMolecule,passiveMolecule);
    // molecule newMol = std::make_shared<molecule_pointer( activeMolecule);
    result.emplace(result.begin(),activeMolecule);
    return result;
}

opResult r_copy(const molecule_pointer mol){
    opResult result;
    if (mol->vector.size()> 2){
        return result;
    }
    molecule_pointer newMol = std::make_shared<molecule>();
    molecule_pointer newMol2 = std::make_shared<molecule>();
    newMol->vector.insert(newMol->vector.begin(), mol->vector.begin()+1,mol->vector.end());
    newMol2->vector.insert(newMol2->vector.begin(), mol->vector.begin()+1,mol->vector.end());


    result.push_back(newMol);
    result.push_back(newMol2);
    return result;
}

opResult r_fork(const molecule_pointer mol){
    opResult result;

    if (mol->vector.size()<2){ return result;}
    molecule_pointer mol1 = std::make_shared<molecule>();
    
    if (mol->vector.size()<3){
        mol1->vector.insert(mol1->vector.begin(),mol->vector.begin()+1,mol->vector.end());
        result.push_back(mol1);
        return result;
    }
    molecule_pointer mol2 = std::make_shared<molecule>();
    if (mol->vector.size()<4){
        mol1->vector.push_back(mol->vector[1]);
        mol2->vector.push_back(mol->vector[2]);
    }
    else{
        mol1->vector.push_back(mol->vector[1]);
        mol1->vector.insert(mol1->vector.end(),mol->vector.begin()+2,mol->vector.end());
        mol2->vector.push_back(mol->vector[2]);
        mol2->vector.insert(mol2->vector.end(),mol->vector.begin()+2,mol->vector.end());
    }
    result.push_back(mol1);
    result.push_back(mol2);

    return result;
}
opResult r_dup(const molecule_pointer mol){
    opResult result;
    if (mol->vector.size()<2){return result;}
    molecule_pointer mol1 = std::make_shared<molecule>();
    if (mol->vector.size()<3){
        mol1->vector.push_back(mol->vector[1]);
        result.push_back(mol1);
        return result;
    }
    mol1->vector.push_back(mol->vector[1]);
    mol1->vector.push_back(mol->vector[2]);
    mol1->vector.insert(mol1->vector.begin(),mol->vector.begin()+2,mol->vector.end());
    return result;
}


opResult r_nop(const molecule_pointer mol){
    opResult result;

    if (mol->vector.size()<2){
        return result;
    }
    molecule_pointer mol1 = std::make_shared<molecule>();
    mol1->vector.insert(mol1->vector.begin(),mol->vector.begin()+1,mol->vector.end());
    result.push_back(mol1);
    return result;
}

opResult r_nul(const molecule_pointer mol){
    opResult result;
    return result;
}
opResult r_split(const molecule_pointer mol){
    opResult result;
    if(mol->vector.size()<2){
        return result;
    }
    molecule_pointer mol1 = std::make_shared<molecule>();
    molecule_pointer mol2 = std::make_shared<molecule>();
    int which = true;
    bool skip =true;
    for (auto frag: mol->vector){
        if (skip){
            skip = false;
            continue;};
        if (*frag == "*"){
            if(which){
                which = false;
                continue;
            }
        }
        if(which){
            mol1->vector.push_back(frag);
        }else{
            mol2->vector.push_back(frag);
        }
    }
    result.push_back(mol1);
    result.push_back(mol2);
    return result;
}


opResult r_exch(const molecule_pointer mol){
    opResult result;

    if(mol->vector.size()<2){
        return result;
    }
    molecule_pointer mol1 = std::make_shared<molecule>();
    if(mol->vector.size()<4){
        mol1->vector.insert(mol1->vector.begin(),mol->vector.begin()+1,mol->vector.end());
        result.push_back(mol1);
        return result;
    }
    if (mol->vector.size()<5){
        mol1->vector.push_back(mol->vector[1]);
        mol1->vector.push_back(mol->vector[3]);
        mol1->vector.push_back(mol->vector[2]);
        result.push_back(mol1);
        return result;
    }
    mol1->vector.push_back(mol->vector[1]);
    mol1->vector.push_back(mol->vector[3]);
    mol1->vector.push_back(mol->vector[2]);
    mol1->vector.insert(mol1->vector.end(),mol->vector.begin()+4,mol->vector.end());
    result.push_back(mol1);
    return result;

}
opResult r_send(const molecule_pointer mo){
    opResult result;
    return result;
}

opResult r_empty(const molecule_pointer mol){
    opResult result;
    std::vector<molecule_pointer> t;
    
    // if ( mol->size() < 3 ){
    //     return result;
    // }

    molecule_pointer mol1 = std::make_shared<molecule>();
    // const molecule_pointer mol2 = mol;

    if (mol->vector.size() == 3){
        // mol1->vector.push_back((*mol).vector[1]);
        // mol->vector.erase(mol->vector.begin());
        // mol->vector.erase(mol->vector.end());
    }

    if (mol->vector.size() > 3){
        mol1->vector.insert(mol1->vector.begin(),mol->vector.begin()+2,mol->vector.end());
        // mol->vector.erase(mol->vector.begin(),mol->vector.begin()+2);
    }
    // t.push_back(mol1);
    result.push_back(mol1);


    return result;
}


opResult r_length(const molecule_pointer mol){
    opResult result;
    if (mol->vector.size() <= 2){
        return result;
    }
    molecule_pointer mol1 = std::make_shared<molecule>();
    std::shared_ptr<symbol> molSize = std::make_shared<symbol>(std::to_string(mol->vector.size()-2));

    mol1->vector.insert(mol1->vector.begin(),mol->vector.begin()+2,mol->vector.end());

    mol1->vector.emplace(mol1->vector.begin(),molSize);
    mol1->vector.emplace(mol1->vector.begin(),mol->vector[1]);
    result.push_back(mol1);
    return result;
}


opResult r_lessthan(const molecule_pointer mol){
    opResult result;

    if (mol->vector.size() > 3){
        std::shared_ptr<symbol> num1 = mol->vector[3];
        std::shared_ptr<symbol> num2 = mol->vector[4];
        if (isNumber(*num1) and isNumber(*num2)){
            float n1  = std::stoi(*num1);
            float n2 = std::stoi(*num2);
            molecule_pointer mol1 = std::make_shared<molecule>();
            if (n1<n2){
                mol1->vector.push_back(mol->vector[1]);
            }
            else{
                mol1->vector.push_back(mol->vector[2]);
            }

            mol1->vector.insert(mol1->vector.end(),mol->vector.begin()+3,mol->vector.end());
            result.push_back(mol1);
        }
    }
    return result;
}

opResult r_pop(const molecule_pointer mol){
    opResult result;
    if (mol->vector.size() < 2){
        return result;
    }
    molecule_pointer mol1 = std::make_shared<molecule>();
    if( mol->vector.size() < 4){
        mol1->vector.push_back(mol->vector[1]);
        result.push_back(mol1);
        return result;
    }
    mol1->vector.push_back(mol->vector[1]);
    mol1->vector.insert(mol1->vector.end(),mol->vector.begin()+3,mol->vector.end());
    result.push_back(mol1);
    return result;

}

opResult r_pop2(const molecule_pointer mol){
    opResult result;

    if (mol->vector.size() < 3){
        return result;
    }
    molecule_pointer mol1 = std::make_shared<molecule>();
    molecule_pointer mol2= std::make_shared<molecule>();
    if( mol->vector.size()== 3){

        mol1->vector.push_back(mol->vector[1]);
        mol2->vector.push_back(mol->vector[2]);
    }
    if (mol->vector.size()> 3){
        mol1->vector.push_back(mol->vector[1]);
        mol1->vector.push_back(mol->vector[3]);
        mol2->vector.push_back(mol->vector[2]);
        mol2->vector.insert(mol2->vector.end(),mol->vector.begin()+4,mol->vector.end());

    }
    result.push_back(mol1);
    result.push_back(mol2);
    return result;
}


std::unordered_map<std::string,bimolOp>  const bimolOpMap = {{match,r_match},{matchp,r_matchp},{perm,r_perm}};
std::unordered_map<std::string,unimolOp> const unimolOpMap = {{dup,r_dup},
                                                      {exch,r_exch},
                                                      {pop,r_pop},
                                                      {nop,r_nop},
                                                    //   {nul,r_nul},
                                                      {split,r_split},
                                                    //   {send,r_send},
                                                      {fork,r_fork},
                                                      {empty,r_empty},
                                                      {length,r_length},
                                                      {lt,r_lessthan},
                                                      {pop,r_pop},
                                                      {pop2, r_pop2},
                                                      {copy,r_copy}
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




void fraglets::inject(const molecule_pointer mol,int mult){
    if (mol->vector.empty() or mult < 1){return;}
    if (mol->vector.size()>= 1){
        if (this->isbimol(mol)){
                const molecule_pointer newMol = this->makeUniqueActive(mol);
                std::shared_ptr<symbol> key = newMol->vector[1];
                // could check for invalid fraglets here.
                this->active.inject(key,newMol,mult);
                this->idle = false;
            }
        else if (this->isunimol(mol)){
            const molecule_pointer newMol = this->makeUniqueUnimol(mol);
            this->unimol.inject(newMol,mult);
            }
        else{
            const molecule_pointer newMol = this->makeUniqueActive(mol);
            std::shared_ptr<symbol> key = newMol->vector[0];
            // could check for invalid fraglets here.
            this->passive.inject(key,newMol,mult);
            this->idle = false;
        }
    }
    // else{
    //     if ((!this->isbimol(mol)) and (!this->isunimol(mol))){
    //     this->passive.inject(mol,mol,mult);
    //     this->idle = false;
    //     }
    // }
}

bool fraglets::isbimol(const molecule_pointer mol){
    if (mol->vector.size() < 2){ return false;}
    std::shared_ptr<symbol> tag = mol->vector[0];
    return (*tag == match) or (*tag == matchp) or (*tag == perm);
}

bool fraglets::isperm(const molecule_pointer mol){
    if (mol->vector.empty()){ return false;}

    std::shared_ptr<symbol> tag = mol->vector[0];
    return (*tag == perm);
}

bool fraglets::isMatchp(const molecule_pointer mol){
    if (mol->vector.empty()){ return false;}

    std::shared_ptr<symbol> tag = mol->vector[0];
    return *tag == matchp;


}
bool fraglets::isunimol(const molecule_pointer mol){

    if (mol->vector.empty()){ return false;}
    std::shared_ptr<symbol> head = mol->vector[0];
    return (unimolTags.find(*head) != unimolTags.end()) & !this->isbimol(mol);
}

double fraglets::propensity(){

    this->run_unimol();

    this->prop.clear();
    this->wt = 0;
    keyMultisetMap::iterator it = this->active.keyMap.begin();
    for (;it != this->active.keyMap.end();it++){
        symbol key = it->first;
        std::size_t m = this->active.multk(key);
        std::size_t p = this->passive.multk(key);
        std::size_t w = m*p;
        // std::cout << key << m << p << '\n';
        if (w > 0){
            this->prop[key] = w;
        }
        this->wt += w;
    }
    if (this->wt <= 0){
        this->idle = true;}
    return this->wt;

}

void fraglets::react(double w){
        // """ perform the selected reaction pointed to by the dice position w
        //     (typically involked from the hierarchical Gillespie SSA
        // """
        if (this->wt < 0){ return;}
        keyMultisetMap::iterator it = this->active.keyMap.begin();
        for (;it != this->active.keyMap.end();it++){
            symbol key = it->first;
            propMapIterator pit = this->prop.find(key);
            if(pit != this->prop.end()){
                double propValue = pit->second;
                if ((propValue > 0) and (w < propValue)){
                    // std::cout << "active \n";
                    const molecule_pointer activeMolecule = this->active.expelrnd(key);
                    // std::cout << "passive \n";
                    const molecule_pointer passiveMolecule = this->passive.expelrnd(key);
                    opResult result = this->react2(activeMolecule,passiveMolecule);
                    this->inject_list(result);
                    return;
            }
            w -= this->prop[key];
        }
    }
}


opResult fraglets::react1(const molecule_pointer mol){

    std::shared_ptr<symbol> tag = mol->vector[0];
    opResult result;
    unimolOp f = unimolOpMap.find(*tag)->second; 
    result = f(mol);
    return result;
}

opResult fraglets::react2(const molecule_pointer activeMolecule,const molecule_pointer passiveMolecule ){
    auto x = *activeMolecule->vector.begin();

    std::shared_ptr<symbol> tag = activeMolecule->vector[0];
    opResult result;
    bimolOp f = bimolOpMap.find(*tag)->second;
    result = f(activeMolecule,passiveMolecule);


    if (result.size() == 1){

        std::cout << "[ " <<  molToString(activeMolecule) << "] ,  [ " <<  molToString(passiveMolecule) << "]  --> \n[ " <<  molToString(result[0]) << "]\n";
        this->addEdge(activeMolecule,result[0],false,isMatchp(activeMolecule));
        this->addEdge(passiveMolecule,result[0],false,false);

    }
    if (result.size() == 2){

        std::cout << "[ " <<  molToString(activeMolecule) << "] ,  [ " <<  molToString(passiveMolecule) << "]  --> \n[ " <<  molToString(result[0]) << "] ,  [ " <<  molToString(result[1]) << "]\n";
        this->addEdge(activeMolecule,result[0],false,isMatchp(activeMolecule));
        this->addEdge(activeMolecule,result[1],false,isMatchp(activeMolecule));
        this->addEdge(passiveMolecule,result[0],false,false);
        this->addEdge(passiveMolecule,result[1],false,false);
    }
    return result;
}

int fraglets::run_unimol(){
    int n = 0;
    while (!this->unimol.multiset.empty()){
        const molecule_pointer mol = this->unimol.expelrnd();

        opResult result = this->react1(mol);
        const char* const delim = " ";
        if (result.size() == 1){


            std::cout << "[ " << molToString(mol) << "]  --> \n[ " << molToString(result[0]) << "]\n" ;
            this->addEdge(mol,result[0],true,false);
        }
        if (result.size() == 2){

            std::cout << "[ " << molToString(mol) << "]  --> \n[ " << molToString(result[0]) << "] ,  [ " <<  molToString(result[1]) << "]\n" ;
            this->addEdge(mol,result[0],true,false);
            this->addEdge(mol,result[1],true,false);
        }
        this->inject_list(result);
        n++;
    }
    return n;
}


void fraglets::run_bimol(){
    if (this->wt <= 0){return;}
    std::cout << "test2\n";
    double w = random_double() * this->wt;
    this->react(w);
}


void fraglets::inject_list(opResult result){
    opResult::iterator it = result.begin();
    for (;it!=result.end();it++){
        const molecule_pointer mol = *it;
        this->inject(mol);
        this->reactionCoutTable.inject(mol,1);
    }
}


void fraglets::iterate(){
    this->propensity();
    std::cout << "test\n";
    if (!this->idle){
        this->run_bimol();
    }
    this->activeMultisetSize.push_back(this->active.total);
    this->passiveMultisetSize.push_back(this->passive.total);

}


void fraglets::run(int niter,int molCap){
    for (int i = 1;i<niter;i++){
        // this->trace();
        std::cout<< "ITER= "<<i<<'\n';
        this->iterate();
        int total = this->active.total + this->passive.total;



        // std::map<molecule,int> molCountMap;


        // for (auto activeKey :this->active.keyMap){
        //     moleculeMultiset mset = *activeKey.second;
        //     for (auto mol : mset.multiset){
        //         int mult = mset.mult(mol);
        //         auto mapMolIt = this->mappedMols.find(mol);
        //         if (mapMolIt == this->mappedMols.end()){
        //             this->stackplotIndexMap[this->stackplotIndexCounter] = mol;
        //             this->mappedMols.insert(mol);
        //             this->stackplotIndexCounter += 1;
        //         }
        //         molCountMap[mol] = mult;
        //     }
        // }
        // for (auto passiveKey :this->passive.keyMap){
        //     moleculeMultiset mset = *passiveKey.second;
        //     for (auto mol : mset.multiset){
        //         int mult = mset.mult(mol);
        //         auto mapMolIt = this->mappedMols.find(mol);
        //         if (mapMolIt == this->mappedMols.end()){
        //             this->stackplotIndexMap[this->stackplotIndexCounter] = mol;
        //             this->mappedMols.insert(mol);
        //             this->stackplotIndexCounter += 1;
        //         }
        //         molCountMap[mol] = mult;
        //     }
        // }
        // for (auto unimol :this->unimol.multiset){
        //     int mult = this->unimol.mult(unimol);
        //     auto mapMolIt = this->mappedMols.find(unimol);
        //     if (mapMolIt == this->mappedMols.end()){
        //         this->stackplotIndexMap[this->stackplotIndexCounter] = unimol;
        //         this->mappedMols.insert(unimol);
        //         this->stackplotIndexCounter += 1;
        //     }
        //     molCountMap[unimol] = mult;

        // }


        // while (this->stackplotIndexCounter > this->StackplotVector.size()){
        //     std::vector<int> molvec;
        //     this->StackplotVector.push_back(molvec);
        // }
        // for(auto mappedMol : stackplotIndexMap){
        //     molecule mol = mappedMol.second;
        //     int mult = molCountMap[mol];
        //     this->StackplotVector[mappedMol.first].push_back(mult);
        // }
        // for (auto vIt : this->StackplotVector){
        //     while (vIt.size() < this->stackplotIndexCounter ){
        //         vIt.push_back(0);
        //     }
        // }


        // while (total > molCap){
        //     int n = rand() % 2;
        //     if (n){
        //         if (this->active.total > 0){
        //             keyMultisetMap::iterator random_it = std::next(std::begin(this->active.keyMap), rand_between(0, this->active.keyMap.size()-1));
        //             molecule mol = this->active.expelrnd(random_it->first);
        //             if (isMatchp(mol)){
        //                 this->inject(mol);
        //             }
        //             }
        //         }
        //         else if (this->passive.total > 0){
        //             keyMultisetMap::iterator random_it = std::next(std::begin(this->passive.keyMap), rand_between(0, this->passive.keyMap.size()-1));
        //             this->passive.expelrnd(random_it->first);
        //         }
        //         total = this->active.total + this->passive.total;
        // }
        if (this->idle){
            this->drawGraphViz();
            std::cout<< "idle\n";
            return;
        }
    }
    this->drawGraphViz();
    std::cout<< "done\n";
    return;
}



void fraglets::drawGraphViz(){

    // for(auto edge : this->edgeTable){

    //     auto t =edge.first;
    //     int reactionCount = ((this->reactionCoutTable.mult()/(this->reactionCoutTable.mult(t)+1)))+1;

    //     std::string s = std::to_string(reactionCount);
    //     char  *weight = const_cast<char *>(s.c_str());
    //     agsafeset(edge.second,"weight",weight,weight);
    //     agsafeset(edge.second,"penwidth",weight,weight);
    // }

    GVC_t* graphContext = gvContext();

    gvLayout(graphContext,this->graph,"dot");
    // char* args[] = {
    //     "-Gconcentrate=true"
    // };
    // gvParseArgs (graphContext, sizeof(args)/sizeof(char*), args);
    agsafeset(this->graph,"concentrate","true","true");
    gvRenderFilename(graphContext,this->graph,"pdf","fraglets_map.pdf");
    gvRenderFilename(graphContext,this->graph,"dot","fraglets_map.dot");
    gvFreeLayout(graphContext, this->graph);
    agclose(this->graph);

    gvFreeContext(graphContext);
}


// std::shared_ptr<symbol> test(symbol sym){
//     return std::make_shared<symbol>(sym);
// }


void fraglets::parse(std::string line){


    if ((line[0] == '#') or line.empty()){
        return;
    }

    // if (bracket2 != line.back()){

    // }

    size_t bracket1 = line.find_first_of("[");
    size_t bracket2 = line.find_first_of("]");

    
    symbol _mol = symbol(line.substr(bracket1+1,bracket2-(bracket1+1)));

    std::stringstream iss(_mol);

    molecule molVector = molecule();

    std::istream_iterator<symbol> test{iss},end;
    std::vector<symbol> tempMol(test,end);
    for (symbol sym : tempMol){      
        auto newSymbol =  std::make_shared<symbol>(sym);  
        molVector.vector.push_back(newSymbol);
    }

    // molecule_pointer mol= molecule_pointer(&molVector);
    auto mol = std::make_shared<molecule>(molVector);
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
    // std::cout << "================================\n";
    // keyMultisetMap::iterator ait = this->active.keyMap.begin();
    // for (;ait!=this->active.keyMap.end();ait++){
    //     moleculeMultiset amset = *ait->second;
    //     unorderedMultiset::iterator amit = amset.multiset.begin();
    //     for(;amit!=amset.multiset.end();amit++){
    //         // std::cout << *amit << '\n';
    //     }
    // }
    // keyMultisetMap::iterator pit = this->passive.keyMap.begin();
    // for (;pit!=this->passive.keyMap.end();pit++){
    //     moleculeMultiset pmset = *pit->second;
    //     unorderedMultiset::iterator pmit = pmset.multiset.begin();
    //     for(;pmit!=pmset.multiset.end();pmit++){
    //         // std::cout << *pmit << '\n';
    //     }
    // }
    // std::cout << "================================\n";
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


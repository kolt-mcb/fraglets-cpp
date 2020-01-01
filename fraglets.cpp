#include "fraglets.h"
#include <chrono>
#include <sstream>
#include <iterator>
#include <cstddef>
#include <fstream>
#include <string>
#include <sstream>
#include <iterator>



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
std::string pop2   = "pop2";
std::string copy   = "copy";

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

symbol molToString(molecule mol){
    const char* const delim = " ";
    std::string nodeString;

    for (auto sym : mol){
        nodeString.append(*sym);
        nodeString.push_back(' ');
    }

    return nodeString;


}


std::unordered_set<std::string> bimolTags = {match,matchp};
std::unordered_set<std::string> unimolTags = {exch,pop,nop,split,length,fork,empty,pop2,copy,lt};



void fraglets::addNode(const std::string mol,const bool& unimol,const bool& matchp,const bool& bimol){
    char* c_mol = new char[mol.length() + 1];

    // mol.copy(c_mol,mol.size(),mol.front());
    std::copy(mol.begin(),mol.end(),c_mol);
    c_mol[mol.size()] = '\0';
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

void fraglets::addEdge(const std::shared_ptr<molecule> mol,const std::shared_ptr<molecule> resultMol,const bool& unimol,const bool& matchp){


    std::string molString = molToString(*mol);

    std::string resultMolString =  molToString(*resultMol);



    if (this->nodesTable.find(molString) == this->nodesTable.end()){
        this->addNode(molString,isunimol(mol),isMatchp(mol),isbimol(mol));
    }
    if (this->nodesTable.find(resultMolString) == this->nodesTable.end()){
        this->addNode(resultMolString,isunimol(resultMol),isMatchp(resultMol),isbimol(resultMol));
    }
    Agnode_t* tailNode = nodesTable[resultMolString];
    Agnode_t* headNode = nodesTable[molString];
    Agedge_t* edge = agedge(this->graph,headNode,tailNode,"",true);
    if (unimol){
        agsafeset(edge,"color","blue","blue");
    }else if (matchp){
        agsafeset(edge,"color","green","green");
        agsafeset(edge,"dir","none","none");
    }else{
        agsafeset(edge,"color","black","black");
    }
    this->edgeTable[molString] = edge;
}
opResult r_match(const std::shared_ptr<molecule> activeMolecule, const std::shared_ptr<molecule> passiveMolecule){
    opResult result;



    std::shared_ptr<molecule> newMol =  std::make_shared<molecule>(activeMolecule->begin()+2,activeMolecule->end());
    newMol->insert(newMol->end(),passiveMolecule->begin()+1,passiveMolecule->end());


    result.insert(result.begin(),newMol);
    return result;
}


opResult r_matchp(const std::shared_ptr<molecule> activeMolecule, const std::shared_ptr<molecule> passiveMolecule){
    opResult result =  r_match(activeMolecule,passiveMolecule);
    // std::shared_ptr<molecule> newMol = std::make_shared<molecule>(activeMolecule);
    result.push_back(activeMolecule);
    return result;
}

opResult r_copy(const std::shared_ptr<molecule> mol){
    opResult result;
    if (mol->size()> 2){
        return result;
    }
    result.push_back(std::make_shared<molecule>(mol->begin()+1,mol->end()));
    result.push_back(std::make_shared<molecule>(mol->begin()+1,mol->end()));
    return result;
}

opResult r_fork(const std::shared_ptr<molecule> mol){
    opResult result;

    if (mol->size()<2){ return result;}
    std::shared_ptr<molecule> mol1 = std::make_shared<molecule>();
    if (mol->size()<3){
        mol1->insert(mol1->begin(),mol->begin()+1,mol->end());
        result.push_back(mol1);
        return result;
    }
    std::shared_ptr<molecule> mol2 = std::make_shared<molecule>();
    if (mol->size()<4){
        mol1->push_back((*mol)[1]);
        mol2->push_back((*mol)[2]);
    }
    else{
        mol1->push_back((*mol)[1]);
        mol1->insert(mol1->end(),mol->begin()+2,mol->end());
        mol2->push_back((*mol)[2]);
        mol2->insert(mol2->end(),mol->begin()+2,mol->end());
    }
    result.push_back(mol1);
    result.push_back(mol2);

    return result;
}
opResult r_dup(const std::shared_ptr<molecule> mol){
    opResult result;
    if (mol->size()<2){return result;}
    std::shared_ptr<molecule> mol1 = std::make_shared<molecule>();
    if (mol->size()<3){
        mol1->push_back((*mol)[1]);
        result.push_back(mol1);
        return result;
    }
    mol1->push_back((*mol)[1]);
    mol1->push_back((*mol)[2]);
    mol1->insert(mol1->begin(),mol->begin()+2,mol->end());
    return result;
}


opResult r_nop(const std::shared_ptr<molecule> mol){
    opResult result;

    if (mol->size()<2){
        return result;
    }
    std::shared_ptr<molecule> mol1 = std::make_shared<molecule>();
    mol1->insert(mol1->begin(),mol->begin()+1,mol->end());
    result.push_back(mol1);
    return result;
}

opResult r_nul(const std::shared_ptr<molecule> mo){
    opResult result;
    return result;
}
opResult r_split(const std::shared_ptr<molecule> mol){
    opResult result;
    if(mol->size()<2){
        return result;
    }
    std::shared_ptr<molecule> mol1 = std::make_shared<molecule>();
    std::shared_ptr<molecule> mol2 = std::make_shared<molecule>();
    int which = true;
    bool skip =true;
    for (auto frag: *mol){
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
            mol1->push_back(frag);
        }else{
            mol2->push_back(frag);
        }
    }
    result.push_back(mol1);
    result.push_back(mol2);
    return result;
}


opResult r_exch(const std::shared_ptr<molecule> mol){
    opResult result;

    if(mol->size()<2){
        return result;
    }
    std::shared_ptr<molecule> mol1 = std::make_shared<molecule>();
    if(mol->size()<4){
        mol1->insert(mol1->begin(),mol->begin()+1,mol->end());
        result.push_back(mol1);
        return result;
    }
    if (mol->size()<5){
        mol1->push_back((*mol)[1]);
        mol1->push_back((*mol)[3]);
        mol1->push_back((*mol)[2]);
        result.push_back(mol1);
        return result;
    }
    mol1->push_back((*mol)[1]);
    mol1->push_back((*mol)[3]);
    mol1->push_back((*mol)[2]);
    mol1->insert(mol1->end(),mol->begin()+4,mol->end());
    result.push_back(mol1);
    return result;

}
opResult r_send(const std::shared_ptr<molecule> mo){
    opResult result;
    return result;
}

opResult r_empty(const std::shared_ptr<molecule> mol){
    opResult result;

    if ( mol->size() < 3 ){
        return result;
    }
    std::shared_ptr<molecule> mol1 = std::make_shared<molecule>();
    if (mol->size() == 3){
        mol1->push_back((*mol)[1]);
    }
    if (mol->size() > 3){
        mol1->insert(mol1->begin(),mol->begin()+2,mol->end());
    }
    result.push_back(mol1);
    return result;
}


opResult r_length(const std::shared_ptr<molecule> mol){
    opResult result;
    if (mol->size() <= 2){
        return result;
    }
    std::shared_ptr<molecule> mol1 = std::make_shared<molecule>();
    std::shared_ptr<symbol> molSize = std::make_shared<symbol>(std::to_string(mol->size()-2));

    mol1->insert(mol1->begin(),mol->begin()+2,mol->end());

    mol1->emplace(mol1->begin(),molSize);
    mol1->emplace(mol1->begin(),(*mol)[1]);
    result.push_back(mol1);
    return result;
}


opResult r_lessthan(const std::shared_ptr<molecule> mol){
    opResult result;

    if (mol->size() > 3){
        std::shared_ptr<symbol> num1 = (*mol)[3];
        std::shared_ptr<symbol> num2 = (*mol)[4];
        if (isNumber(*num1) and isNumber(*num2)){
            float n1  = std::stoi(*num1);
            float n2 = std::stoi(*num2);
            std::shared_ptr<molecule> mol1 = std::make_shared<molecule>();
            if (n1<n2){
                mol1->push_back((*mol)[1]);
            }
            else{
                mol1->push_back((*mol)[2]);
            }

            mol1->insert(mol1->end(),mol->begin()+3,mol->end());
            result.push_back(mol1);
        }
    }
    return result;
}

opResult r_pop(const std::shared_ptr<molecule> mol){
    opResult result;
    if (mol->size() < 2){
        return result;
    }
    std::shared_ptr<molecule> mol1 = std::make_shared<molecule>();
    if( mol->size() < 4){
        mol1->push_back((*mol)[1]);
        result.push_back(mol1);
        return result;
    }
    mol1->push_back((*mol)[1]);
    mol1->insert(mol1->end(),mol->begin()+3,mol->end());
    result.push_back(mol1);
    return result;

}

opResult r_pop2(const std::shared_ptr<molecule> mol){
    opResult result;

    if (mol->size() < 3){
        return result;
    }
    std::shared_ptr<molecule> mol1 = std::make_shared<molecule>();
    std::shared_ptr<molecule> mol2= std::make_shared<molecule>();
    if( mol->size()== 3){

        mol1->push_back((*mol)[1]);
        mol2->push_back((*mol)[2]);
    }
    if (mol->size()> 3){
        mol1->push_back((*mol)[1]);
        mol1->push_back((*mol)[3]);
        mol2->push_back((*mol)[2]);
        mol2->insert(mol2->end(),mol->begin()+4,mol->end());

    }
    result.push_back(mol1);
    result.push_back(mol2);
    return result;
}


std::unordered_map<std::string,bimolOp>  const bimolMap = {{match,r_match},{matchp,r_matchp}};
std::unordered_map<std::string,unimolOp> const unimolMap = {{dup,r_dup},
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




void fraglets::inject(std::shared_ptr<molecule> mol,int mult){
    if (mol->empty() or mult < 1){return;}
    if (mol->size()>= 1){
        if (this->isbimol(mol)){
                std::shared_ptr<symbol> key = (*mol)[1];
                // could check for invalid fraglets here.
                this->active.inject(key,mol,mult);
                this->idle = false;
            }
        else if (this->isunimol(mol)){
            this->unimol.inject(mol,mult);
            }
        else{
            std::shared_ptr<symbol> key = (*mol)[0];
            // could check for invalid fraglets here.
            this->passive.inject(key,mol,mult);
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

bool fraglets::isbimol(const std::shared_ptr<molecule> mol){
    if (mol->empty()){ return false;}

    std::shared_ptr<symbol> tag = (*mol)[0];
    return (*tag == match) or (*tag == matchp);
}

bool fraglets::isMatchp(const std::shared_ptr<molecule> mol){
    if (mol->empty()){ return false;}

    std::shared_ptr<symbol> tag = (*mol)[0];
    return *tag == matchp;


}
bool fraglets::isunimol(const std::shared_ptr<molecule> mol){

    if (mol->empty()){ return false;}
    std::shared_ptr<symbol> head = (*mol)[0];
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
                    std::shared_ptr<molecule> activeMolecule = this->active.expelrnd(key);
                    std::shared_ptr<molecule> passiveMolecule = this->passive.expelrnd(key);
                    opResult result = this->react2(activeMolecule,passiveMolecule);
                    opResult::iterator rIt = result.begin();
                    this->inject_list(result);
                    return;
            }
            w -= this->prop[key];
        }
    }
}


opResult fraglets::react1(std::shared_ptr<molecule> mol){

    std::shared_ptr<symbol> tag = (*mol)[0];
    opResult result;
    unimolOp f = unimolMap.find(*tag)->second; 
    result = f(mol);
    return result;
}

opResult fraglets::react2(std::shared_ptr<molecule> activeMolecule,std::shared_ptr<molecule> passiveMolecule ){

    std::shared_ptr<symbol> tag = (*activeMolecule)[0];
    opResult result;
    bimolOp f = bimolMap.find(*tag)->second;
    result = f(activeMolecule,passiveMolecule);


    if (result.size() == 1){

        std::cout << "[ " <<  molToString(*activeMolecule) << " ] , [ " <<  molToString(*passiveMolecule) << " ] -> \n[ " <<  molToString(*result[0]) << " ]\n";
        this->addEdge(activeMolecule,result[0],false,isMatchp(activeMolecule));
        this->addEdge(passiveMolecule,result[0],false,false);

    }
    if (result.size() == 2){

        std::cout << "[ " <<  molToString(*activeMolecule) << " ] , [ " <<  molToString(*passiveMolecule) << " ] -> \n[ " <<  molToString(*result[0]) << " ] , [ " <<  molToString(*result[1]) << " ]\n";
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

        std::shared_ptr<molecule> mol = this->unimol.expelrnd();
        opResult result = this->react1(mol);
        const char* const delim = " ";
        if (result.size() == 1){


            std::cout << "[ " << molToString(*mol) << " ] ->\n[ " << molToString(*result[0]) << " ]\n" ;
            this->addEdge(mol,result[0],true,false);
        }
        if (result.size() == 2){

            std::cout << "[ " << molToString(*mol) << " ] ->\n[ " << molToString(*result[0]) << " ] , [ " <<  molToString(*result[1]) << " ]\n" ;
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
    double w = random_double() * this->wt;
    this->react(w);
}


void fraglets::inject_list(opResult result){
    opResult::iterator it = result.begin();
    for (;it!=result.end();it++){
        std::shared_ptr<molecule> mol = *it;
        this->inject(mol);
        // this->reactionCoutTable.inject(mol,1);
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


void fraglets::run(int niter,int molCap){
    for (int i = 1;i<niter;i++){
        // this->trace();
        std::cout<< "ITER="<<i<<'\n';
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
            // this->drawGraphViz();
            return;
        }
    }
    // this->drawGraphViz();
    return;
}



void fraglets::drawGraphViz(){

        // for(auto edge : this->edgeTable){
    //     int reactionCount = ((this->reactionCoutTable.mult({""})/this->reactionCoutTable.mult(edge.first)))+1;

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

    std::shared_ptr<molecule> mol(new molecule());

    std::istream_iterator<symbol> test{iss},end;
    std::vector<symbol> tempMol(test,end);

    for (symbol sym : tempMol){
        std::shared_ptr<symbol> newSymbol(new symbol(sym));
        mol->push_back(newSymbol);
    }

        
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


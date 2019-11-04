#include "moleculemultiset.h"
#include <random>
#include <iostream>


int rand_between(int begin,int end){

    std::random_device rd;
    std::mt19937 mt(rd());
    std::uniform_int_distribution<int> dist(begin, end);
    return dist(mt);

}




void moleculeMultiset::inject(const molecule& mol,int mult = 1){
    // if (this->multiset.find(mol) == this->multiset.end()){
    //     this->multiset[mol] = 1;
    // }

    // for (int i = 1; i < mult; i++){
    //     this->multiset[mol]++;
    // }
    for (int i = 0; i< mult; i++){
        this->multiset.insert(mol);
        // std::cout<<"insert " << mol << '\n';
    }
}
int moleculeMultiset::expel(const molecule& mol, int mult = 1){

    // int total = 0;
    
    // int remaning = this->multiset[mol];
    // if (remaning <= mult){
    //     this->multiset.erase(mol);
    //     return remaning;
    // } 
    // else{
    //     this->multiset[mol] = remaning - mult;
    //     return mult;
    // }
    int total = 0;
    unorderedMultiset::iterator it;
    for (int i = 0; i < mult; i++){
        it = this->multiset.find(mol);
        if (it != this->multiset.end()){
            this->multiset.erase(it);
            total++;
        }
    }
    return total;
}


// https://stackoverflow.com/questions/27024269/select-random-element-in-an-unordered-map
molecule moleculeMultiset::rndMol(){


    if (this->multiset.empty()){
        return "";
    }else{

        // unorderedMultiset::iterator it = this->multiset.begin();
        // molecule mol = *it;
        // std::cout<< "rndmol " << mol << " " << this->multiset.size() << "\n";
        // return mol;
        unorderedMultiset::iterator random_it = std::next(std::begin(this->multiset), rand_between(0, this->multiset.size()-1));
        molecule mol = *random_it;
        return mol;
    }
}
    

molecule moleculeMultiset::expelrnd(){
    molecule mol = this->rndMol();
    this->expel(mol);
    return mol;
}


int moleculeMultiset::mult(molecule mol){
    if (mol.empty()){
        return this->multiset.size();
    }

    return this->multiset.count(mol);
    
}

int moleculeMultiset::mult(){
    return this->multiset.size();
    
}

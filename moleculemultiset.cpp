#include "moleculemultiset.h"
#include <random>
#include <iostream>
#include <memory>


int rand_between(int begin,int end){

    std::random_device rd;
    std::mt19937 mt(rd());
    std::uniform_int_distribution<int> dist(begin, end);
    return dist(mt);

}





void moleculeMultiset::inject(std::shared_ptr<molecule> mol,int mult = 1){
    for (int i = 0; i< mult; i++){
        this->multiset.insert(mol);
    }
}
int moleculeMultiset::expel(std::shared_ptr<molecule> mol, int mult = 1){

    int total = 0;
    unorderedMultiset::iterator it;
    for (int i = 0; i < mult; i++){
        // symbol s = (*this->multiset.begin())[0];
        // symbol s2 = mol[0];
        it = this->multiset.find(mol);
        if (it != this->multiset.end()){
            this->multiset.erase(it);
            total++;
        }
    }
    return total;
}


// https://stackoverflow.com/questions/27024269/select-random-element-in-an-unordered-map
std::shared_ptr<molecule> moleculeMultiset::rndMol(){


    if (this->multiset.empty()){
        // I know this is fucked but what do I do.
        std::shared_ptr<molecule> mol;
        return mol;
    }else{

        // unorderedMultiset::iterator it = this->multiset.begin();
        // molecule mol = *it;
        // std::cout<< "rndmol " << mol << " " << this->multiset.size() << "\n";
        // return mol;

        unorderedMultiset::iterator random_it = std::next(std::begin(this->multiset), rand_between(0, this->multiset.size()-1));
        std::shared_ptr<molecule> mol = *random_it;
        return mol;
    }
}


std::shared_ptr<molecule> moleculeMultiset::expelrnd(){
    std::shared_ptr<molecule> mol = this->rndMol();
    this->expel(mol);
    return mol;
}


int moleculeMultiset::mult(std::shared_ptr<molecule> mol){
    if (mol->empty()){
        return this->multiset.size();
    }

    return this->multiset.count(mol);

}

int moleculeMultiset::mult(){
    return this->multiset.size();

}


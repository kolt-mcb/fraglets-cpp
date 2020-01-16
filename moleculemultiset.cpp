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





moleculeMultiset::moleculeMultiset(){}
moleculeMultiset::~moleculeMultiset(){}

void moleculeMultiset::inject(molecule_pointer mol,int mult = 1){
    for (int i = 0; i< mult; i++){
        this->multiset.insert(mol);
    }
}
int moleculeMultiset::expel(const molecule_pointer mol, int mult = 1){

    int total = 0;
    unorderedMultiset::iterator it;
    for (int i = 0; i < mult; i++){
        // symbol s = (*this->multiset.begin())[0];
        // symbol s2 = mol[0];
        it = this->multiset.find(mol);
        // std::cout << "expel find " << mol << " " << *it  << (it == this->multiset.end())<< '\n';
        if (it != this->multiset.end()){
            this->multiset.erase(mol);
            total++;
        }
        else{
            std::cout<<"error mol\n";
            exit(0);
        }
    }
    return total;
}


// https://stackoverflow.com/questions/27024269/select-random-element-in-an-unordered-map
const molecule_pointer moleculeMultiset::rndMol(){

    
    // if (this->multiset.empty()){
    //     // I know this is fucked but what do I do.
    //     std::cout << "=====================================================================================\n";
    //     molecule mol;
    //     return mol;
    // }else{

        // unorderedMultiset::iterator it = this->multiset.begin();
        // molecule mol = *it;
        // std::cout<< "rndmol " << mol << " " << this->multiset.size() << "\n";
        // return mol;

        // for (auto r : this->multiset){
        //     for (auto k : *r.mol_ptr){
        //         std::cout << *k << " ";
        //     }
        //     std::cout << r.mol_ptr  << "\n";
        // }


        unorderedMultiset::iterator random_it = this->multiset.begin();
        // 'advance' the iterator n times
        std::advance(random_it,rand_between(0, this->multiset.size()-1));



        // for (auto r : this->multiset){
        //     for (auto k : *r.mol_ptr){
        //         std::cout << *k << " ";
        //     }
        //     std::cout << r.mol_ptr  << "\n";
        // }
        // unorderedMultiset::iterator random_it = std::next(std::begin(this->multiset), rand_between(0, this->multiset.size()-1));
        // molecule mol = *random_it;
        // if (this->multiset.find(mol) == this->multiset.end()){
        //     for (auto i : *mol.mol_ptr){
        //         std::cout << *i << '\n';
        //     }
        //     std::cout << mol.mol_ptr<< "test fail \n";
        //     exit(0);
        // }

        auto t = *random_it;
        return *random_it;
    }
// }


const molecule_pointer moleculeMultiset::expelrnd(){
    const molecule_pointer mol = this->rndMol();
    this->expel(mol);
    return mol;
}


int moleculeMultiset::mult(molecule_pointer& mol){
    if (mol->vector.empty()){
        return this->multiset.size();
    }

    return this->multiset.count(mol);

}

int moleculeMultiset::mult(){
    return this->multiset.size();

}

molecule::molecule(){
    // std::cout << "constuct\n";
};
molecule::~molecule(){
    // std::cout << "destuct\n";
};

bool molecule::operator==(const molecule& other) const{
    std::equal(this->vector.begin(), this->vector.end(), other.vector.begin(),
    [](const std::shared_ptr<symbol>& item1, const std::shared_ptr<symbol>
    & item2) -> bool{
        return (*item1 == *item2);
    }); 
}
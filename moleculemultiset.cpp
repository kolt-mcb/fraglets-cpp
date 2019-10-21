#include "moleculemultiset.h"
#include <random>

int rand_between(int begin,int end){

    std::random_device rd;
    std::mt19937 mt(rd());
    std::uniform_real_distribution<float> dist(begin, end);

    return dist(mt);

}


unorderedMultiset multiset;

// moleculeMultiset::moleculeMultiset(){
//     multiset = unorderedMultiset();
// }
void moleculeMultiset::inject(molecule mol,int mult = 0){
    for (int i = 0; i < mult; i++){
        this->multiset.insert(&mol);
    }
    
}
int moleculeMultiset::expel(molecule mol, int mult = 0){
    unorderedMultiset::iterator it = this->multiset.find(&mol);
    int total = 0;
    for (int i = 0;i<mult;i++){
        if (it != this->multiset.end()){
            this->multiset.erase(it);
            it++;
            total++;
        }
    }
    return total;
}


// https://stackoverflow.com/questions/27024269/select-random-element-in-an-unordered-map
molecule moleculeMultiset::rndMol(){
    unorderedMultiset::iterator random_it = std::next(std::begin(this->multiset), rand_between(0, this->multiset.size()));
    molecule mol = **random_it;
    return mol;
}
    

molecule moleculeMultiset::expelrnd(){
    molecule mol = this->rndMol();
    this->expel(mol);
    return mol;
}



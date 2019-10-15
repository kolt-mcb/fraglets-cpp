#include "moleculemultiset.h"

class moleculeMultiset{
    unorderedMultiset multiset;

    moleculeMultiset(){
        multiset = unorderedMultiset();
    }
    void moleculeMultiset::inject(molecule molecule,int mult = 0){
        for (int i = 0; i < mult; i++){
            this->multiset.insert(&molecule);
        }
        
    }
}
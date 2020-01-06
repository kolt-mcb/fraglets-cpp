#include <unordered_map>
#include <vector>
#include <unordered_set>
#include <set>
#include <iostream>
#include <random>
#include <memory>


int rand_between(int begin, int end);

typedef std::string symbol;
typedef std::shared_ptr<std::vector<std::shared_ptr<symbol>>> molecule_pointer;
typedef std::vector<molecule> moleculeVector;


class molecule {
    public:
        std::shared_ptr<std::vector<std::shared_ptr<symbol>>> mol_ptr;

        bool operator()(const molecule first)
        {   
            // std::cout << first.mol_ptr << " " << "\n";
            return !std::equal(this->mol_ptr->begin(), this->mol_ptr->end(), first.mol_ptr->begin(),
                [](std::shared_ptr<symbol> item1, std::shared_ptr<symbol> item2) -> bool{
                std::cout<< "wtf" << *item1 << " " << *item2 <<(*item1 == *item2)<<'\n';
                return (*item1 == *item2);
            });
        }
};


// struct ptr_compare 
// {
//     bool operator()(const std::shared_ptr<molecule> first,const std::shared_ptr<molecule> second)
//     {   
//         std::cout << first << " " << second << "\n";
//         return !equal(first->begin(), first->end(), second->begin(),
//             [](std::shared_ptr<symbol> item1, std::shared_ptr<symbol> item2) -> bool{
//             std::cout<< "wtf" << *item1 << " " << *item2 <<(*item1 == *item2)<<'\n';
//             return (*item1 == *item2);
//         });
//     }
// };

typedef std::unordered_multiset<molecule>   unorderedMultiset;


class moleculeMultiset {

    public:
        // moleculeMultiset();
        void inject(molecule& mol, int mul);
        int expel(molecule& mol, int mult);
        molecule& expelrnd();
        molecule& rndMol();
        int mult(molecule mol);
        int mult();
        unorderedMultiset multiset;
};
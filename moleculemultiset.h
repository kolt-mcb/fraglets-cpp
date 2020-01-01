#include <unordered_map>
#include <vector>
#include <unordered_set>
#include <set>
#include <iostream>
#include <random>
#include <memory>


int rand_between(int begin, int end);

typedef std::string symbol;
typedef std::vector<std::shared_ptr<symbol>> molecule;
typedef std::vector<molecule> moleculeVector;


struct ptr_compare 
{
    bool operator()(std::shared_ptr<molecule> first, std::shared_ptr<molecule> second) const
    {   
        return !equal(first->begin(), first->end(), second->begin(),
            [](const std::shared_ptr<symbol>& item1, const std::shared_ptr<symbol>& item2) -> bool{
            return (*item1 == *item2);
        });
    }
};


typedef std::multiset<std::shared_ptr<molecule>,ptr_compare>   unorderedMultiset;


class moleculeMultiset {

    public:
        // moleculeMultiset();
        void inject(std::shared_ptr<molecule> mol, int mul);
        int expel(std::shared_ptr<molecule> mol, int mult);
        std::shared_ptr<molecule> expelrnd();
        std::shared_ptr<molecule> rndMol();
        int mult(std::shared_ptr<molecule> mol);
        int mult();
        unorderedMultiset multiset;
};
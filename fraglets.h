
#include <unordered_set>
#include <set>
#include <map>
#include <vector> 


typedef std::vector<std::string> molecule;
typedef std::unordered_multiset<molecule*>   molecule_multiset;
typedef std::map<std::string,molecule_multiset*>  keyMultiset;
typedef keyMultiset::iterator keyMultisetIterator;


typedef std::map<std::string, float> prop_map;


// nested_unordered_multiset testset1;
// string_unordered_multiset testset2 = {"test","test","test2"};

// typedef unordered_multiset<unordered_multiset<string>*>::iterator numit;
// typedef unordered_multiset<string>::iterator umit;

// class ops {
//     public:
//         static string match;// = "match";
//         static string matchp;// = "match";

// };
// string ops::match = "match";
// string ops::matchp = "matchp";

std::set<std::string> ops = {"match","matchp","fork","nop"};

class fraglets {
    private:
        keyMultiset active, passive;
        molecule_multiset unimol;
        // ops ops;
        prop_map prop;
        float wt;
        bool idle;
    public:
        void inject(std::vector<std::string> molecule,int mult);
        float propensity();
        void run_unimol();
        bool isbimol(std::vector<std::string> molecule);
        bool isunimol(std::vector<std::string> molecule);
        

};


#include <unordered_set>
#include <set>
#include <map>
#include <vector> 
#include "keymultiset.h"


typedef std::map<std::string, float> propMap;

typedef std::map<std::string, float>::iterator propMapIterator;


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
        moleculeMultiset unimol;
        // ops ops;
        propMap prop;
        float wt;
        bool idle;
    public:
        void inject(molecule molecule,int mult);
        float propensity();
        void run_unimol();
        bool isbimol(molecule molecule);
        bool isunimol(molecule molecule);
        void react(float w);
        

};

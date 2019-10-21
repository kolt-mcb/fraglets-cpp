
#include <unordered_set>
#include <set>
#include <map>
#include <vector> 
#include "keymultiset.h"
#include <functional>


typedef std::map<std::string, float> propMap;

typedef std::map<std::string, float>::iterator propMapIterator;
typedef std::vector<molecule> opResult;
typedef std::function<opResult (molecule)> op ; 
// typedef opResult (*op)(molecule);

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
        void inject(molecule mol,int mult);
        float propensity();
        int run_unimol();
        bool isbimol(molecule mol);
        bool isunimol(molecule mol);
        void react(float w);
        opResult react1(molecule molecule);
        std::vector<molecule> react2(molecule activeMolecule ,molecule passiveMolecule);
        opResult match(molecule activeMolecule ,molecule passiveMolecule);
        opResult fork(molecule molecule);
        void inject_list(opResult);
        

};

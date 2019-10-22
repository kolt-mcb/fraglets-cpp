#ifndef FRAGLETS_H
#define FRAGLETS_H

#include <unordered_set>
#include <set>
#include <map>
#include <vector> 
#include "keymultiset.h"
#include <functional>
#include <unordered_map>


typedef std::map<std::string, double> propMap;

typedef std::map<std::string, double>::iterator propMapIterator;
typedef std::vector<molecule> opResult;
typedef std::function<opResult (molecule,molecule)> bimolOp ; 
typedef std::function<opResult (molecule)> unimolOp ;


extern std::string match;
extern std::string matchp;
extern std::string dup;
extern std::string exch;
extern std::string pop;
extern std::string nop;
extern std::string nul;
extern std::string split;
extern std::string send;
extern std::string fork;

extern std::unordered_set<std::string> bimolTags;
extern std::unordered_set<std::string> unimolTags;


class fraglets {
    private:
        keyMultiset active, passive;
        moleculeMultiset unimol;
        // ops ops;
        propMap prop;
        double wt;
        bool idle;
    public:
        void inject(molecule mol,int mult=1);
        double propensity();
        int run_unimol();
        bool isbimol(molecule mol);
        bool isunimol(molecule mol);
        void react(double w);
        opResult react1(molecule molecule);
        std::vector<molecule> react2(molecule activeMolecule ,molecule passiveMolecule);
        void inject_list(opResult);
        void iterate();
        void run(int niter);
        bool inert();
        void run_bimol();
        

};

#endif
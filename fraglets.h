#ifndef FRAGLETS_H
#define FRAGLETS_H

#include "keymultiset.h"
#include <map>
#include <vector> 
#include <functional>
#include <graphviz/gvc.h>


typedef std::map<std::string, double> propMap;

typedef std::map<std::string, double>::iterator propMapIterator;
typedef std::vector<molecule_pointer> opResult;
typedef std::function<opResult (const molecule_pointer,const molecule_pointer)> bimolOp;
typedef std::function<opResult (const molecule_pointer)> unimolOp;


extern std::string match;
extern std::string matchp;
extern std::string _dup;
extern std::string exch;
extern std::string pop;
extern std::string nop;
extern std::string nul;
extern std::string split;
extern std::string send;
extern std::string _fork;
extern std::string empty;
extern std::string length;
extern std::string lt;
extern std::string pop2;
extern std::string copy;


extern std::unordered_set<std::string> bimolTags;
extern std::unordered_set<std::string> unimolTags;


class fraglets {
    private:
        moleculeMap activeMap, passiveMap, unimolMap;
        keyMultiset active, passive;
        moleculeMultiset unimol;
        // ops ops;
        propMap prop;
        int wt;
        bool idle;
        Agraph_t* graph = agopen("G", Agdirected, NULL);
        Agraph_t* subgraph = agsubg(graph, "cluster", 1);
        std::map <symbol,Agnode_t*> nodesTable;
        std::map <symbol,Agedge_t*> edgeTable;
        std::map<int,molecule_pointer>  stackplotIndexMap;
        std::set<molecule_pointer> mappedMols;
        int stackplotIndexCounter = 1;
        std::unordered_multiset<symbol> reactionCoutTable;
        void addNode(symbol mol,const bool& unimol,const bool& matchp,const bool& bimol);
        void addEdge(molecule_pointer activeMolecule,const molecule_pointer passiveMolecule,const bool& unimol,const bool& matchp);
        const molecule_pointer makeUniqueUnimol(const molecule_pointer);
        const molecule_pointer makeUniqueActive(const molecule_pointer);
        const molecule_pointer makeUniquePassive(const molecule_pointer);
        void inject_list(opResult);
        void react(double w);
        opResult react1(const molecule_pointer mol);
        opResult react2(const molecule_pointer activeMolecule ,const molecule_pointer passiveMolecule);
        void iterate();
        bool inert();
        std::vector<int> activeMultisetSize;
        std::vector<int> passiveMultisetSize;
        void run_bimol();
        std::vector<std::vector<int>> StackplotVector;
        void inject(const molecule_pointer mol,int mult=1);
        double propensity();
        int run_unimol();
        bool quiet;

        

    public:
        bool isbimol(const molecule_pointer mol);
        bool isperm(const molecule_pointer mol);
        bool isMatchp(const molecule_pointer mol);
        bool isunimol(const molecule_pointer mol);
        void run(int niter,int molCap,bool quiet);
        void parse(std::string line);
        void interpret(std::string filename);
        void trace();
        void drawGraphViz();
        int iter = 0;
        


        

};

#endif
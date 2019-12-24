#ifndef FRAGLETS_H
#define FRAGLETS_H

#include "keymultiset.h"
#include <map>
#include <vector> 
#include <functional>
#include <QtWidgets/QApplication>
#include <QtWidgets/QMainWindow>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <graphviz/gvc.h>



typedef std::map<std::string, double> propMap;

typedef std::map<std::string, double>::iterator propMapIterator;
typedef std::vector<const molecule*> opResult;
typedef std::function<opResult (const molecule&, const molecule&)> bimolOp;
typedef std::function<opResult (const molecule&)> unimolOp;


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
extern std::string empty;
extern std::string length;
extern std::string lt;
extern std::string pop2;
extern std::string copy;


extern std::unordered_set<std::string> bimolTags;
extern std::unordered_set<std::string> unimolTags;


class fraglets {
    private:
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
        std::map<int,molecule>  stackplotIndexMap;
        std::set<molecule> mappedMols;
        int stackplotIndexCounter = 1;
        moleculeMultiset reactionCoutTable;
        void addNode(const std::string& mol,const bool& unimol,const bool& matchp,const bool& bimol);
        void addEdge(const molecule& activeMolecule,const molecule& passiveMolecule,const bool& unimol,const bool& matchp);

        


    public:
        std::vector<std::vector<int>> StackplotVector;
        void inject(const molecule* mol,int mult=1);
        double propensity();
        int run_unimol();
        bool isbimol(const molecule& mol);
        bool isMatchp(const molecule& mol);
        bool isunimol(const molecule& mol);
        void react(double w);
        opResult react1(const molecule& mol);
        opResult react2(const molecule& activeMolecule ,const molecule& passiveMolecule);
        void inject_list(opResult);
        void iterate();
        void run(int niter,int molCap);
        bool inert();
        void run_bimol();
        void show_plot();
        std::vector<int> activeMultisetSize;
        std::vector<int> passiveMultisetSize;
        void parse(std::string line);
        void interpret(std::string filename);
        void trace();
        void drawGraphViz();


        

};

#endif

#include <unordered_set>
#include <set>
#include <map>
#include <vector> 
using namespace std;




typedef map<string,unordered_multiset<vector<string>*>*>  keymultiset;
typedef unordered_multiset<vector<string>*>   molecule_multiset;
typedef std::map<std::string, float> prop;

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

set<string> ops = {"match","matchp","fork","nop"};

class fraglets {
    private:
        keymultiset active, passive;
        molecule_multiset unimol;
        // ops ops;
        // prop prop;
        float wt;
        bool idle;
    public:
        void inject(vector<string> molecule,int mult);
        bool isbimol(vector<string> molecule);
        bool isunimol(vector<string> molecule);
        

};

        // self.op fraglets::= { # implementation of instruction set
        //     'M' : [ self.r_match,  'match' ],
        //     'Z' : [ self.r_matchp, 'matchp' ],
        //     'D' : [ self.r_dup,    'dup' ],
        //     'E' : [ self.r_exch,   'exch' ],
        //     'P' : [ self.r_pop,    'pop' ],
        //     'N' : [ self.r_nop,    'nop' ],
        //     'U' : [ self.r_nul,    'nul' ],
        //     'S' : [ self.r_split,  'split' ],
        //     'X' : [ self.r_send,   'send' ],
        //     'F' : [ self.r_fork,   'fork' ]
        // }
//  self.prop = {}
//         self.wt = 0.0
//         self.cnx = {} # list of remote connections, identified by a tag
//         self.nodeid = nid # tag that identifies this node
//         self.idle = Tru
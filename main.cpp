#include <iostream>
#include <unordered_set>
// #include "unordered_multiset.cpp"
#include "fraglets.cpp"

using namespace std;


// typedef unordered_multiset<unordered_multiset<string>*>  nested_unordered_multiset;
// typedef unordered_multiset<string>   string_unordered_multiset;

// nested_unordered_multiset testset1;
// string_unordered_multiset testset2 = {"test","test","test2"};

// typedef unordered_multiset<unordered_multiset<string>*>::iterator numit;
// typedef unordered_multiset<string>::iterator umit;

// void printUset(nested_unordered_multiset ums)
// {
//     //  begin() returns iterator to first element of set
//     numit it = ums.begin();
//     for (; it != ums.end(); it++){
//         umit nested_iterator = (*it)->begin();
//         for (; nested_iterator != (*it)->end(); nested_iterator++ ){
//             cout << *nested_iterator << "";
//         }
//     }
//     cout << endl;
// }





int main() {


    // testset1.insert(&testset2);
    // // std:cout << testset1.count(&testset2);
    // // printUset(testset1);
    // cout << testset2.count("tes23123t") << "";


    fraglets frag;
    vector<string> x ;

    x.push_back("jdnfjnf");
    x.push_back("x");
    cout << frag.isbimol(x);




    return 0;
}
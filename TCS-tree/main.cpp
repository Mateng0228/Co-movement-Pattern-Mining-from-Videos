#include <iostream>
#include "query.h"

using namespace std;
int main(int argc, char *argv[]) {
    // get input parameters
    if(argc - 1 < 4){
        cerr<<"please input the correct parameters: dataset, m, k, eps"<<endl;
        return -1;
    }
    string dataset(argv[1]);
    long long m = stoll(argv[2]);
    long long k = stoll(argv[3]);
    double eps = stod(argv[4]);
    set<string> OPTIONS{"no-tcs", "cmc", "no-hash", "details"}, ADDITIONS;
    for(int arg_i = 5; arg_i < argc; arg_i++){
        if(OPTIONS.find(argv[arg_i]) == OPTIONS.end()){
            cerr<<"unrecognized optional parameters!"<<endl;
            return -1;
        }
        else ADDITIONS.insert(argv[arg_i]);
    }
    cout<<"parameters{"<<"dataset:"<<dataset<<", m:"<<m<<", k:"<<k<<", epsilon:"<<eps<<"}, additional options[ ";
    if(ADDITIONS.empty()) cout<<" ";
    else{
        string addition_string;
        for(auto &addition : ADDITIONS) addition_string += addition + ",";
        addition_string.back() = ' ';
        cout<<addition_string;
    }
    cout<<"]:"<<endl;

    // query
    TCS_Query tcs_query(dataset, m, k, eps, ADDITIONS);
    tcs_query.query();

    return 0;
}
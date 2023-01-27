#include <iostream>
#include "algorithm1/cmc_query.h"
#include "algorithm2/st_query.h"

using namespace std;
int main(int argc, char *argv[]) {
    // get input parameters
    if(argc - 1 != 4){
        cerr<<"please input the correct parameters: dataset, m, k, eps"<<endl;
        return -1;
    }
    string dataset(argv[1]);
    long long m = stoll(argv[2]);
    long long k = stoll(argv[3]);
    double eps = stod(argv[4]);
    // query
//    CMC_Query cmc_query(dataset, m, k, eps);
//    cmc_query.query();

    ST_Query st_query(dataset, m, k, eps);
    st_query.query();

    return 0;
}
#include <iostream>
#include <unordered_set>
#include <unordered_map>
#include <random>
#include <map>
#include <deque>
#include <chrono>
#include "input_handler.h"
#include "baseline.h"
#include "baseline_ext.h"
#include "miner.h"

using namespace std;

void test_offline(string dataset, int m, int k, double eps){
    InputHandler handler("Datasets/" + dataset);
    vector<pair<int, Position>> &update_items = handler.get_update_data();
    map<int, Path> &data_map = handler.get_history_map();

    auto start = chrono::high_resolution_clock::now();
    TCS_Query tcs_query(data_map, m, k, eps);
    OfflineResult* result= tcs_query.query();
    result->print_contents();
    delete result;
    auto end = chrono::high_resolution_clock::now();
    chrono::duration<double> elapsed = end - start;
    cout << "TCS-tree:" << elapsed.count() << "s" <<endl;

    start = chrono::high_resolution_clock::now();
    TreeMiner miner(m, k, eps);
    for(const auto &update_item : update_items){
        miner.update(update_item.first, update_item.second);
    }
    miner.print_result();
    end = chrono::high_resolution_clock::now();
    elapsed = end - start;
    cout << "TCS-trie:" << elapsed.count() << "s" <<endl;
}

void test_property(string dataset, int m = 3, int k = 5, double eps = 60, int size = 50000){
    InputHandler handler("Datasets/" + dataset);
    vector<pair<int, Position>> &update_items = handler.get_update_data(size);

//    Baseline baseline(m, k, eps);
//    vector<double> baseline_latencies;
//    auto baseline_begin_time = chrono::high_resolution_clock::now();
//    for(const auto &update_item : update_items){
//        auto start = chrono::high_resolution_clock::now();
//        baseline.update(update_item.first, update_item.second);
//        auto end = chrono::high_resolution_clock::now();
//        chrono::duration<double, std::milli> duration = end - start;
//        baseline_latencies.push_back(duration.count());
//    }
//    auto baseline_end_time = chrono::high_resolution_clock::now();
//    chrono::duration<double> baseline_duration = baseline_end_time - baseline_begin_time;
//    cout<<"["<<dataset<<","<<m<<","<<k<<","<<eps<<","<<size<<"] "<<"Result of Baseline: ";
//    cout<<"latency("<<accumulate(baseline_latencies.begin(),baseline_latencies.end(),0.0) / static_cast<double>(baseline_latencies.size())<<"), ";
//    cout<<"throughput("<<(size / baseline_duration.count())<<"), ";
//    cout<<"Time(" << baseline_duration.count()<<")"<< endl;

//    Baseline_Ext baseline_ext(m, k, eps);
//    vector<double> be_latencies;
//    auto be_begin_time = chrono::high_resolution_clock::now();
//    for(const auto &update_item : update_items){
//        auto start = chrono::high_resolution_clock::now();
//        baseline_ext.update(update_item.first, update_item.second);
//        auto end = chrono::high_resolution_clock::now();
//        chrono::duration<double, std::milli> duration = end - start;
//        be_latencies.push_back(duration.count());
//    }
//    auto be_end_time = chrono::high_resolution_clock::now();
//    chrono::duration<double> be_duration = be_end_time - be_begin_time;
//    cout<<"["<<dataset<<","<<m<<","<<k<<","<<eps<<","<<size<<"] "<<"Result of Baseline_Ext: ";
//    cout<<"latency("<<accumulate(be_latencies.begin(),be_latencies.end(),0.0) / static_cast<double>(be_latencies.size())<<"), ";
//    cout<<"throughput("<<(size / be_duration.count())<<"), ";
//    cout<<"Time(" << be_duration.count()<<")"<< endl;
//    cout<<tot_tree<<" | "<<tot_pattern<<" | "<<tot_dominance<<endl;

//    TreeMiner miner_sp(m, k, eps);
//    vector<double> sp_latencies;
//    auto sp_begin_time = chrono::high_resolution_clock::now();
//    for(const auto &update_item : update_items){
//        auto start = chrono::high_resolution_clock::now();
//        miner_sp.update(update_item.first, update_item.second, false);
//        auto end = chrono::high_resolution_clock::now();
//        chrono::duration<double, std::milli> duration = end - start;
//        sp_latencies.push_back(duration.count());
//    }
//    auto sp_end_time = chrono::high_resolution_clock::now();
//    chrono::duration<double> sp_duration = sp_end_time - sp_begin_time;
//    cout<<"["<<dataset<<","<<m<<","<<k<<","<<eps<<","<<size<<"] "<<"Result of TCS-trie w/o: ";
//    cout<<"latency("<<accumulate(sp_latencies.begin(),sp_latencies.end(),0.0) / static_cast<double>(sp_latencies.size())<<"), ";
//    cout<<"throughput("<<(size / sp_duration.count())<<"), ";
//    cout<<"Time(" << sp_duration.count()<<")"<< endl;
//    cout<<num_affected<<endl;

    TreeMiner miner(m, k, eps);
    vector<double> trie_latencies;
    auto trie_begin_time = chrono::high_resolution_clock::now();
    for(const auto &update_item : update_items){
        auto start = chrono::high_resolution_clock::now();
        miner.update(update_item.first, update_item.second);
        auto end = chrono::high_resolution_clock::now();
        chrono::duration<double, std::milli> duration = end - start;
        trie_latencies.push_back(duration.count());
    }
    auto trie_end_time = chrono::high_resolution_clock::now();
    chrono::duration<double> trie_duration = trie_end_time - trie_begin_time;
    cout<<"["<<dataset<<","<<m<<","<<k<<","<<eps<<","<<size<<"] "<<"Result of TCS-trie: ";
    cout<<"latency("<<accumulate(trie_latencies.begin(),trie_latencies.end(),0.0) / static_cast<double>(trie_latencies.size())<<"), ";
    cout<<"throughput("<<(size / trie_duration.count())<<"), ";
    cout<<"Time(" << trie_duration.count()<<")"<< endl;
    cout<<num_affected<<endl;
};

int main(int argc, char* argv[]){
    string dataset = argv[1];
    int m = std::stoi(argv[2]);
    int k = std::stoi(argv[3]);
    double eps = std::stoi(argv[4]);
    int size = 50000;
    if(argc > 5) size = std::stoi(argv[5]);

    test_property(dataset, m, k, eps, size);
//    test_offline(dataset, m, k, eps);

    return 0;
}
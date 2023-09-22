#ifndef PROJECT_QUERY_H
#define PROJECT_QUERY_H

#include <string>
#include <vector>
#include "algorithm/position.h"
#include "algorithm/cluster.h"
#include "algorithm/tree.h"
#include "algorithm/mining_base.h"
#include "algorithm/mining_sw.h"
#include "algorithm/result.h"
#include "utils.h"

using namespace std;
typedef long long ll;

class TCS_Query{
private:
    bool window_flag = true; // whether to use the sliding window based verification

    void query(MiningTree &mining_tree, ll node_id, vector<int> &sub2complete, Result &result);
    void suppress_tcs(vector<position> *arr_positions, int arr_size);
public:
    string dataset;
    ll m;
    ll k;
    double eps;
    set<string> ADDITIONS;

    TCS_Query(string dataset, ll m, ll k, double eps, set<string> ADDITIONS){
        this->dataset = dataset;
        this->m = m;
        this->k = k;
        this->eps = eps;
        this->ADDITIONS = ADDITIONS;
        if(ADDITIONS.find("cmc") != ADDITIONS.end()) window_flag = false;
    }

    void query();
};

void TCS_Query::query() {
    clock_t begin_time = clock();

    // Data initialization
    vector<string> data_paths;
    get_subfiles("datasets/" + dataset, data_paths);
    int arr_size = data_paths.size();
    auto *arr_positions = new vector<position>[arr_size];
    fill_data(arr_positions, data_paths);

    // compute corresponding temporal cluster and meta-cluster information for each car
    clock_t record_time = clock();
    add_cluster_mark(arr_positions, arr_size, m, eps);
    if(ADDITIONS.find("no-tcs") != ADDITIONS.end()) suppress_tcs(arr_positions, arr_size);
    cout<<"cluster: "<<static_cast<double>(clock() - record_time) / CLOCKS_PER_SEC<<" | ";

    // trajectory simplification (based on the meta-cluster information, remove irrelevant positions from the camera sequence of each vehicle)
    record_time = clock();
    vector<vector<position>> positions_list;
    vector<int> sub2complete;
    trajectory_simplification(arr_positions, arr_size, k, positions_list, sub2complete);

    if(positions_list.empty()) cout<<"Total number of discovered patterns: 0, object combinations: 0"<<endl;
    else{
        // construct a TCS-tree or standard suffix tree for pattern mining
        MiningTree mining_tree(positions_list);
        cout<<"build tree: "<<static_cast<double>(clock() - record_time) / CLOCKS_PER_SEC<<" | ";

        // obtain the un-duplicated result set containing non-maximal patterns
        record_time = clock();
        Result *result = nullptr;
        if(ADDITIONS.find("no-hash") != ADDITIONS.end()) result = new ResultBaseImpl();
        else result = new ResultTwoMapImpl(mining_tree);
        query(mining_tree, mining_tree.root, sub2complete, *result);
        cout<<"verification: "<<static_cast<double>(clock()-record_time)/CLOCKS_PER_SEC - insert_time/CLOCKS_PER_SEC<<" | ";

        // deduplicate to obtain all maximal patterns
        record_time = clock();
        result->de_duplication();
        cout<<"de_duplicate: "<<static_cast<double>(clock()-record_time)/CLOCKS_PER_SEC + insert_time/CLOCKS_PER_SEC<<" | ";

        // output the final result
        if(ADDITIONS.find("details") != ADDITIONS.end()){
            result->dump_contents(mining_tree, "results/output.csv");
        }
        else result->print_contents();
        delete result;
    }
    delete[] arr_positions;

    cout<<"Total elapsed time: "<<static_cast<double>(clock() - begin_time) / CLOCKS_PER_SEC<<"s"<<endl;
}

void TCS_Query::query(MiningTree &mining_tree, ll node_id, vector<int> &sub2complete, Result &result) {
    MiningNode *tree = mining_tree.tree;
    if(tree[node_id].is_leaf) return;

    if(tree[node_id].depth >= k){
        ll common_length = tree[node_id].depth;
        ll leaf_left = tree[node_id].leaf_left, leaf_right = tree[node_id].leaf_right;
        if(leaf_right - leaf_left + 1 < m) return;

        vector<ll> begin_ids;
        vector<int> cars;
        for(ll i = leaf_left; i <= leaf_right; i++){
            MiningNode &leaf_node = tree[mining_tree.leaves[i]];
            ll begin_id = leaf_node.end - leaf_node.depth;
            begin_ids.push_back(begin_id);
            int positions_list_idx = mining_tree.terminator2ListId[mining_tree.text[leaf_node.end - 1].camera];
            cars.push_back(sub2complete[positions_list_idx]);
        }
        int n_cars = count_unique(cars);
        if(n_cars < m) return;

        if(window_flag){
            SW_Miner miner(common_length, begin_ids, cars, mining_tree);
            miner.mine(result, m, k, eps);
        }
        else{
            Base_Miner miner(common_length, begin_ids, cars, mining_tree);
            miner.mine(result, m, k, eps);
        }
    }
    for(auto & entry : tree[node_id].next){
        ll nxt = entry.second;
        query(mining_tree, nxt, sub2complete, result);
    }
}

void TCS_Query::suppress_tcs(vector<position> *arr_positions, int arr_size) {
    /* Here, we will populate all the meta-clusters corresponding to the positions of each camera that all vehicles pass through with the same value 1.
     * This will suppress and eliminate the acceleration effect of subsequent trajectory simplification, TCS-tree construction, and other steps. */
    for(int i = 0; i < arr_size; i++){
        vector<position> &positions = arr_positions[i];
        for(position &pst : positions){
            pst.group = 1;
        }
    }
}

#endif //PROJECT_QUERY_H

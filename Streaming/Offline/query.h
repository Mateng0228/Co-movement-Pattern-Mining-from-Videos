#ifndef PROJECT_QUERY_H
#define PROJECT_QUERY_H

#include "position.h"
#include "cluster.h"
#include "tree.h"
#include "mining_sw.h"
#include "result.h"
#include "../structs.h"

using namespace std;

class TCS_Query{
private:
    void query(MiningTree &mining_tree, ll node_id, vector<int> &sub2complete, OfflineResult &result);
    int count_unique(vector<int>&);
public:
    map<int, Path> &path_map;
    int m, k;
    double eps;

    TCS_Query(map<int, Path> &path_map, int m, int k, double eps): path_map(path_map){
        this->m = m;
        this->k = k;
        this->eps = eps;
    }

    OfflineResult* query();
};

OfflineResult* TCS_Query::query() {
    // Data initialization
    int arr_size = static_cast<int>(path_map.size());
    auto *arr_positions = new vector<offline_position>[arr_size];
    int idx = 0; auto itor = path_map.begin();
    for(; idx < arr_size; ++idx, ++itor){
        arr_positions[idx] = vector<offline_position>();
        Path &path = itor->second;
        for(Position &p : path.positions)
            arr_positions[idx].emplace_back(p.camera, p.ts, p.te);
    }
    add_cluster_mark(arr_positions, arr_size, m, eps);

    // trajectory simplification
    vector<vector<offline_position>> positions_list;
    vector<int> sub2complete;
    trajectory_simplification(arr_positions, arr_size, k, positions_list, sub2complete);

    auto *result = new OfflineResult();
    if(!positions_list.empty()){
        MiningTree mining_tree(positions_list);
        query(mining_tree, mining_tree.root, sub2complete, *result);
        result->de_duplication();
    }
    delete[] arr_positions;
    return result;
}

void TCS_Query::query(MiningTree &mining_tree, ll node_id, vector<int> &sub2complete, OfflineResult &result) {
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

        SW_Miner miner(common_length, begin_ids, cars, mining_tree);
        miner.mine(result, m, k, eps);
    }
    for(auto & entry : tree[node_id].next){
        ll nxt = entry.second;
        query(mining_tree, nxt, sub2complete, result);
    }
}

int TCS_Query::count_unique(vector<int>& list) {
    if(list.empty()) return 0;

    vector<int> temp_list = list;
    sort(temp_list.begin(), temp_list.end());
    int counts = 1, pre_val = temp_list[0];
    for(int i = 1;i < temp_list.size();i++){
        if(temp_list[i] != pre_val){
            counts++;
            pre_val = temp_list[i];
        }
    }
    return counts;
}

#endif //PROJECT_QUERY_H

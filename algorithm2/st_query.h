#ifndef PROJECT_ST_QUERY_H
#define PROJECT_ST_QUERY_H

#include <string>
#include <vector>
#include "position.h"
#include "cluster.h"
#include "tree.h"
#include "st_mining1.h"
#include "st_mining2.h"
#include "../utils.h"
#include "../result.h"

using namespace std;
typedef long long ll;

class ST_Query{
private:
    void query(MiningTree &mining_tree, ll node_id, vector<int> &sub2complete, Result &result);
public:
    string dataset;
    ll m;
    ll k;
    double eps;

    ST_Query(string dataset, ll m, ll k, double eps){
        this->dataset = dataset;
        this->m = m;
        this->k = k;
        this->eps = eps;
    }

    void query();
};
void ST_Query::query() {
    clock_t begin_time = clock();

    // 初始化数据
    vector<string> data_paths;
    get_subfiles("datasets/" + dataset, data_paths);
    int arr_size = data_paths.size();
    auto *arr_positions = new vector<position>[arr_size];
    fill_data(arr_positions, data_paths);

    // 为各车辆的摄像头添加聚类和分组信息
    add_cluster_mark(arr_positions, arr_size, m, eps);
    // 去掉不属于任何聚类的position
    vector<vector<position>> positions_list;
    vector<int> sub2complete;
    split_positions(arr_positions, arr_size, k, positions_list, sub2complete);
    if(positions_list.empty()) cout<<"Total number of discovered convoys: 0, object combinations: 0"<<endl;
    else{
        // 构建后缀树
        MiningTree mining_tree(positions_list);
        // 获得未去重的结果集
        ResultBaseImpl result;
//        ResultTwoMapImpl result(mining_tree);
        query(mining_tree, mining_tree.root, sub2complete, result);
        // 去重
        result.de_duplication();
        // 输出结果
//        result.print_contents();
//        result.dump_contents(mining_tree, "results/base.csv");
    }
    delete[] arr_positions;

    cout<<"total elapsed time: "<<static_cast<double>(clock() - begin_time) / CLOCKS_PER_SEC<<"s"<<endl;
}

void ST_Query::query(MiningTree &mining_tree, ll node_id, vector<int> &sub2complete, Result &result) {
    MiningNode *tree = mining_tree.tree;
    if(tree[node_id].is_leaf) return;

    if(tree[node_id].depth >= k){
        ll common_length = tree[node_id].depth;
        ll leaf_left = tree[node_id].leaf_left, leaf_right = tree[node_id].leaf_right;
        if(leaf_right - leaf_left + 1 < m) return;

        vector<ll> begin_ids;
        vector<int> cars;
        for(ll i = leaf_left; i <= leaf_right; i++){
            MiningNode& leaf_node = tree[mining_tree.leaves[i]];
            ll begin_id = leaf_node.end - leaf_node.depth;
            begin_ids.push_back(begin_id);
            int positions_list_idx = mining_tree.terminator2ListId[mining_tree.text[leaf_node.end - 1].camera];
            cars.push_back(sub2complete[positions_list_idx]);
        }
        int n_cars = count_unique(cars);
        if(n_cars < m) return;

        ST_Miner2 miner(common_length, begin_ids, cars, mining_tree);
        miner.mine(result, m, k, eps);
    }
    for(auto & entry : tree[node_id].next){
        ll nxt = entry.second;
        query(mining_tree, nxt, sub2complete, result);
    }
}

#endif //PROJECT_ST_QUERY_H

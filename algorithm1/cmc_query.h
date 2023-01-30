#ifndef PROJECT_CMC_QUERY_H
#define PROJECT_CMC_QUERY_H

#include <set>
#include "cmc_tree.h"
#include "cmc_mining.h"
#include "../utils.h"

class CMC_Query{
private:
    void query(SuffixTree &st, ll node_id, Result &result);
public:
    string dataset;
    ll m;
    ll k;
    double eps;

    CMC_Query(string dataset, ll m, ll k, double eps){
        this->dataset = dataset;
        this->m = m;
        this->k = k;
        this->eps = eps;
    }

    void query(){
        clock_t begin_time = clock();

        // 初始化数据
        vector<string> data_paths;
        get_subfiles("datasets/" + dataset, data_paths);
        int arr_size = data_paths.size();
        auto *arr_cameras = new vector<ll>[arr_size];
        auto *arr_intervals = new vector<pair<double, double>>[arr_size];
        fill_data(arr_cameras, arr_intervals, data_paths);

        // 构造后缀树
        SuffixTree st(arr_cameras, arr_intervals, arr_size);
        // 初始化保存convoys的数据结构
        ResultBaseImpl result;
        // 遍历并挖掘convoys
        query(st, st.root, result);
        // 结果集去重
        result.de_duplication();
        // 输出convoys结果
//        result.print_contents();
        delete[] arr_cameras, arr_intervals;

        cout<<"total elapsed time: "<<static_cast<double>(clock() - begin_time) / CLOCKS_PER_SEC<<"s"<<endl;
    }
};

void CMC_Query::query(SuffixTree &st, ll node_id, Result &result) {
    Node *tree = st.tree;
    if(tree[node_id].is_leaf) return;

    if(tree[node_id].depth >= k){
        ll commonLength = tree[node_id].depth;
        ll leafStart = tree[node_id].leaf_left, leafEnd = tree[node_id].leaf_right;
        if(leafEnd - leafStart + 1 < m) return;// 叶节点的数目都小于m

        vector<ll> objectPosList;
        vector<int> objectIdList;
        for(ll i = leafStart;i <= leafEnd;i++){
            Node& leafNode = tree[st.leaves[i]];
            ll currStart = leafNode.end - leafNode.depth;
            objectPosList.push_back(currStart);
            objectIdList.push_back(st.terminator2id(st.text[leafNode.end - 1]));
        }
        // 车辆集合的数量小于m
        int n_objects = count_unique(objectIdList);
        if(n_objects < m) return;

        CMC_Miner miner(commonLength, &st, &objectPosList, &objectIdList);
        miner.mine(result, m, k, eps);
    }
    for(auto & it : tree[node_id].next){
        ll nxt = it.second;
        query(st, nxt, result);
    }
}

#endif //PROJECT_CMC_QUERY_H

#ifndef EXPERIMENT_RESULT_H
#define EXPERIMENT_RESULT_H

#include "structs.h"
#include <unordered_set>
#include <iostream>

using RstIterator = unordered_map<vector<int>, list<vector<pair<int, int>>>, hash_vector>::iterator;
class Result{
public:
    static constexpr int BUCKET_SIZE = 100000;
    unordered_map<vector<int>, list<vector<pair<int, int>>>, hash_vector> result_map;
    Result() {result_map.reserve(BUCKET_SIZE);}
    void insert(vector<int> &oids, list<vector<pair<int, int>>> &trajs);
    void print();
private:
    struct hash_rst{
        size_t operator()(const RstIterator& it) const {
            return std::hash<const void*>{}(&(*it));
        }
    };
    struct equal_rst{
        bool operator()(const RstIterator& a, const RstIterator& b) const {
            return &(*a) == &(*b);
        }
    };
    unordered_map<int, unordered_set<RstIterator, hash_rst, equal_rst>> inverted_index;

    void remove_nonmax(const vector<int> &ref_oids, list<vector<pair<int, int>>> &ref_trajs, const vector<int> &oids, list<vector<pair<int, int>>> &trajs);
};

void Result::insert(vector<int> &oids, list<vector<pair<int, int>>> &trajs) {
    // 通过倒排表获得包含与被包含的对象集合
    int n = static_cast<int>(oids.size());
    unordered_map<RstIterator, int, hash_rst, equal_rst> cnt_map;
    for(int oid : oids){
        auto inverted_it = inverted_index.find(oid);
        if(inverted_it != inverted_index.end()){
            auto &rst_iterators = inverted_it->second;
            for(const RstIterator &rst_iterator : rst_iterators){
                auto cnt_it = cnt_map.find(rst_iterator);
                if(cnt_it == cnt_map.end()) cnt_map.insert(make_pair(rst_iterator, 1));
                else cnt_it->second += 1;
            }
        }
    }
    vector<RstIterator> super_iterators, sub_iterators;
    for(auto &cnt_entry : cnt_map){
        const RstIterator &rst_iterator = cnt_entry.first;
        int m = static_cast<int>(rst_iterator->first.size());
        int cnt = cnt_entry.second;
        if(cnt == n){
            if(m == n) sub_iterators.push_back(rst_iterator);
            super_iterators.push_back(rst_iterator);
        }
        else if(cnt < n && cnt == m) sub_iterators.push_back(rst_iterator);
    }
    // 处理超集
    sort(super_iterators.begin(), super_iterators.end(), [](const RstIterator &it1, const RstIterator &it2){
        return it1->first.size() < it2->first.size();
    });
    for(RstIterator &super_iterator : super_iterators){
        remove_nonmax(super_iterator->first, super_iterator->second, oids, trajs);
        if(trajs.empty()) break;
    }
    if(trajs.empty()) return;
    // 处理子集
    for(RstIterator &sub_iterator : sub_iterators){
        remove_nonmax(oids, trajs, sub_iterator->first, sub_iterator->second);
        if(sub_iterator->second.empty()){
            for(int sub_oid : sub_iterator->first) inverted_index.at(sub_oid).erase(sub_iterator);
            result_map.erase(sub_iterator);
        }
    }
    // 插入
    auto insert_it = result_map.find(oids);
    if(insert_it == result_map.end()){
        const RstIterator &insert_iterator = result_map.insert(make_pair(oids, trajs)).first;
        for(int oid : oids) inverted_index[oid].insert(insert_iterator);
    }
    else insert_it->second.splice(insert_it->second.begin(), trajs);
}

void Result::remove_nonmax(const vector<int> &ref_oids, list<vector<pair<int, int>>> &ref_trajs, const vector<int> &oids, list<vector<pair<int, int>>> &trajs){
    vector<int> indices;
    for(int ref_idx = 0, idx = 0; ref_idx < ref_oids.size(); ++ref_idx){
        if(ref_oids[ref_idx] == oids[idx]){
            indices.push_back(ref_idx);
            idx++;
        }
    }
    if(indices.size() != oids.size()) cerr<<"error in remove_nonmax()!"<<endl;

    for(auto traj_it = trajs.begin(); traj_it != trajs.end();){
        vector<pair<int, int>> &traj = *traj_it;
        bool delete_flag = false;
        for(vector<pair<int, int>> &ref_traj : ref_trajs){
            bool contain_flag = true;
            for(int idx = 0; idx < traj.size(); ++idx){
                pair<int, int> &interval = traj[idx], &ref_interval = ref_traj[indices[idx]];
                if(!(ref_interval.first <= interval.first && ref_interval.second >= interval.second)){
                    contain_flag = false;
                    break;
                }
            }
            if(contain_flag){
                delete_flag = true;
                break;
            }
        }
        if(delete_flag) traj_it = trajs.erase(traj_it);
        else ++traj_it;
    }
}

void Result::print() {
    int n_objects = 0, n_convoys = 0;
    for(auto &entry : result_map){
        if(entry.second.empty()) continue;
        n_objects++;
        n_convoys += static_cast<int>(entry.second.size());
    }
    cout << "Total number of discovered convoys: " << n_convoys << ", object combinations: " << n_objects << endl;
}



#endif //EXPERIMENT_RESULT_H

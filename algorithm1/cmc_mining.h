#ifndef PROJECT_CMC_MINE_H
#define PROJECT_CMC_MINE_H

#include <algorithm>
#include <map>
#include "cmc_tree.h"
#include "group.h"
#include "../result.h"

class CMC_Miner{
private:
    ll sequenceLength;
    SuffixTree *st;
    vector<ll> *objectPosList;// 所有对象在开始时刻所处的位置（在后缀树中的text和time中的位置）
    vector<int> *objectIdList;// 对象名称列表

    void collect_items(map<int, vector<int>>&, map<int, vector<int>>::iterator, vector<int>&, vector<vector<int>>&);
    void insertIntoResults(Result &results, Group &candidate, ll m, ll k);
public:
    CMC_Miner(ll sequenceLength, SuffixTree *st, vector<ll> *objectPosList, vector<int> *objectIdList){
        this->sequenceLength = sequenceLength;
        this->st = st;
        this->objectPosList = objectPosList;
        this->objectIdList = objectIdList;
    }
    // 此方法会完全挖掘所有可能的序列
    void mine_full(Result &result, ll m, ll k, double eps);
    // 此方法只会挖掘开始于第一个位置的所有序列
    void mine(Result &result, ll m, ll k, double eps);
};

void CMC_Miner::mine_full(Result &result, ll m, ll k, double eps) {
    vector<Group> candidates;
    // 以摄像头为单位进行聚类并连通
    for(int idx = 0;idx < sequenceLength;idx++){
        vector<pair<double, double>> intervals;
        for(ll pos : *objectPosList){
            ll currPos = pos + idx;
            intervals.push_back(st->time[currPos]);
        }
        vector<set<int>> clusters;
        MergeSCAN(intervals, eps, m, clusters);
        vector<Group> newCandidates, nextCandidates;
        for(set<int> &cluster : clusters) newCandidates.emplace_back(cluster, false);

//        // candidaates相关辅助数组，只有在聚类得到的集合互相没有交集的前提下使用
//        vector<int> remain_objects;
//        for(Group& group : candidates) remain_objects.push_back(group.cluster.size());
        // newCandidates相关辅助数组
        vector<int> contain_candiates;
        for(int count_idx = 0;count_idx < newCandidates.size();count_idx++) contain_candiates.push_back(0);

        // candidates 与 newCandidates 中的元素进行逐个匹配，从而进行车队发现
        for(int candidate_idx = 0;candidate_idx < candidates.size();candidate_idx++){
            Group& candidate = candidates[candidate_idx];
            candidate.assigned = false;
            for(int nc_idx = 0;nc_idx < newCandidates.size();nc_idx++){
//                if(remain_objects[candidate_idx] == 0) break;// 若当前candidate中的全部元素全部都被发现完毕

                Group& newCandidate = newCandidates[nc_idx];
                set<int> intersectCluster;
                set_intersection(candidate.cluster.begin(), candidate.cluster.end(),
                                 newCandidate.cluster.begin(), newCandidate.cluster.end(),
                                 inserter(intersectCluster, intersectCluster.begin()));
                if(intersectCluster.empty()) continue;
//                remain_objects[candidate_idx] -= intersectCluster.size();
                contain_candiates[nc_idx] += 1;
                if(intersectCluster.size() < m){
                    if(!candidate.assigned){
                        candidate.assigned = true;
                        if(candidate.lifeTime() >= k) insertIntoResults(result, candidate, m, k);
                    }
                }
                else{ // m <= intersectCluster.size() <= candidate.cluster.size()
                    if(intersectCluster.size() == candidate.cluster.size()) candidate.assigned = true;
                    else{// intersectCluster.size() != candidate.cluster.size()
                        if(!candidate.assigned){
                            candidate.assigned = true;
                            if(candidate.lifeTime() >= k) insertIntoResults(result, candidate, m, k);
                        }
                    }
                    newCandidate.assigned = true;
                    nextCandidates.emplace_back(intersectCluster, candidate.startTime, idx);
                }
            }
            if(!candidate.assigned && candidate.lifeTime() >= k) insertIntoResults(result, candidate, m, k);
        }
        for(int nc_idx = 0;nc_idx < newCandidates.size();nc_idx++){
            Group newCandidate = newCandidates[nc_idx];
            if(!newCandidate.assigned)
                nextCandidates.emplace_back(newCandidate.cluster, idx, idx);
            else{
                if(contain_candiates[nc_idx] > 1) nextCandidates.emplace_back(newCandidate.cluster, idx, idx);
            }
        }
        candidates = nextCandidates;
    }
    for(Group& candidate : candidates){
        if(candidate.lifeTime() >= k) insertIntoResults(result, candidate, m, k);
    }
}

void CMC_Miner::mine(Result &result, ll m, ll k, double eps) {
    // 获得各个objects在开始位置上形成的聚类
    vector<pair<double, double>> intervals_first;
    for(ll pos : *objectPosList) intervals_first.push_back(st->time[pos]);
    vector<set<int>> clusters_first;
    MergeSCAN(intervals_first, eps, m, clusters_first);

    vector<Group> candidates;
    for(set<int> &cluster : clusters_first)candidates.emplace_back(cluster, 0, 0, false);
    // 从第2个位置开始逐个聚类并连通来发现convoy
    for(int idx = 1;idx < sequenceLength;idx++){
        vector<pair<double, double>> intervals;
        for(ll pos : *objectPosList) intervals.push_back(st->time[pos + idx]);
        vector<set<int>> clusters;
        MergeSCAN(intervals, eps, m, clusters);
        set<Group> next_candidates;
        for(int candidate_idx = 0;candidate_idx < candidates.size();candidate_idx++){
            Group &candidate = candidates[candidate_idx];
            candidate.assigned = false;
            for(set<int> &cluster : clusters){
                // 取cluster和candidate.cluster的交集
                set<int> intersection;
                set_intersection(candidate.cluster.begin(),candidate.cluster.end(),cluster.begin(),cluster.end(),
                                 inserter(intersection,intersection.begin()));

                if(intersection.size() < m) continue;
                else{
                    if(intersection.size() == candidate.cluster.size()) candidate.assigned = true;
                    next_candidates.insert(Group(intersection, 0, idx, false));
                }
            }
            if(!candidate.assigned && candidate.lifeTime() >= k) insertIntoResults(result, candidate, m, k);
        }
        candidates = vector<Group>(next_candidates.begin(), next_candidates.end());
    }
    for(Group &candidate : candidates){
        if(candidate.lifeTime() >= k) insertIntoResults(result, candidate, m, k);
    }
}

void CMC_Miner::insertIntoResults(Result& result, Group &candidate, ll m, ll k) {
    bool need_collect = false;// 若在同一个聚类内出现了多个相同object，则该聚类有多个可能的结果，最终结果需要被判断和收集
    map<int, vector<int>> object2subscripts;
    for(int subscript : candidate.cluster){
        int object_id = (*objectIdList)[subscript];
        if(object2subscripts.find(object_id) != object2subscripts.end()) need_collect = true;
        else object2subscripts[object_id] = vector<int>();
        object2subscripts[object_id].push_back(subscript);
    }
    if(object2subscripts.size() < m) return;
    vector<vector<int>> subscripts_list;
    if(need_collect){
        vector<int> subscripts;
        collect_items(object2subscripts, object2subscripts.begin(), subscripts, subscripts_list);
    }
    else{
        vector<int> subscripts;
        for(auto & p : object2subscripts){
            subscripts.push_back(p.second.front());
        }
        subscripts_list.push_back(subscripts);
    }

    vector<ll> key;
    for(auto &entry : object2subscripts) key.push_back(entry.first);
    vector<vector<ll>> value;
    for(vector<int> &subscripts : subscripts_list){
        value.emplace_back();
        vector<ll> &insert_list = value.back();
        for(int subscript : subscripts) insert_list.push_back((*objectPosList)[subscript] + candidate.startTime);
    }
    result.insert(key, value, candidate.endTime - candidate.startTime + 1);
}

void CMC_Miner::collect_items(map<int, vector<int>>& object2idx, map<int, vector<int>>::iterator it, vector<int>& subscripts, vector<vector<int>>& subscripts_list){
    if(it == object2idx.end()){
        subscripts_list.push_back(subscripts);
        return;
    }
    for(int subscript : (*it).second){
        subscripts.push_back(subscript);
        collect_items(object2idx, next(it), subscripts, subscripts_list);
        subscripts.pop_back();
    }
}

#endif //PROJECT_CMC_MINE_H
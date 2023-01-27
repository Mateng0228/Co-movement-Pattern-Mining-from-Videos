#ifndef PROJECT_GROUP_H
#define PROJECT_GROUP_H
#include <utility>
#include <vector>
#include <set>
#include <cmath>
#include <algorithm>
#include "cmc_tree.h"

struct Group{
    set<int> cluster;
    int startTime = -1, endTime = -1;
    bool assigned = false;

    Group(set<int>& cluster, bool assigned){
        this->cluster = cluster;
        this->assigned = assigned;
    }
    Group(set<int>& cluster, int startTime, int endTime){
        this->cluster = cluster;
        this->startTime = startTime;
        this->endTime = endTime;
    }
    Group(set<int>& cluster, int startTime, int endTime, bool assigned){
        this->cluster = cluster;
        this->startTime = startTime;
        this->endTime = endTime;
        this->assigned = assigned;
    }

    int lifeTime() const{
        return endTime - startTime + 1;
    }

    bool operator<(const Group& other_group) const{
        if(startTime == other_group.startTime){
            if(endTime == other_group.endTime) return cluster < other_group.cluster;
            else return endTime < other_group.endTime;
        }
        else return startTime < other_group.startTime;
    }
};

bool isClose(pair<double, double> from, pair<double, double> to, double eps){
    return abs(from.first - to.first) < eps;
}
void DBSCAN(vector<pair<double, double>>& intervals, double eps, ll m, vector<set<int>>& clusters){
    int N = intervals.size();
    int arr_status[N];// -1：未遍历，0：离群点，1：属于某聚类
    fill(arr_status, arr_status + N, -1);
    for(int interval_id = 0;interval_id < N;interval_id++){
        if(arr_status[interval_id] != -1)continue;
        //计算邻域内节点的数量
        set<int> neighbors;
        for(int k = 0;k < N;k++){
            if(isClose(intervals[interval_id], intervals[k], eps)){
                neighbors.insert(k);
            }
        }
        if(neighbors.size() < m){
            arr_status[interval_id] = 0;
            continue;
        }
        set<int> cluster;
        arr_status[interval_id] = 1;
        cluster.insert(interval_id);
        neighbors.erase(interval_id);
        set<int> newNeighbors;
        while(!neighbors.empty() || !newNeighbors.empty()){
            set<int>& currNeighbors = neighbors, &otherNeighbors = newNeighbors;
            if(neighbors.empty())swap(currNeighbors, otherNeighbors);
            for(int j : currNeighbors){
                if(arr_status[j] == 0){
                    arr_status[j] = 1;
                    cluster.insert(j);
                }
                if(arr_status[j] != -1)continue;
                arr_status[j] = 1;
                cluster.insert(j);
                set<int> expandNeighbors;
                for(int k = 0;k < N;k++){
                    if(isClose(intervals[j], intervals[k], eps)){
                        expandNeighbors.insert(k);
                    }
                }
                if(expandNeighbors.size() >= m){
                    for(int item : expandNeighbors){
                        if(currNeighbors.find(item) == currNeighbors.end()){
                            otherNeighbors.insert(item);
                        }
                    }
                }
            }
            currNeighbors.clear();
        }
        clusters.emplace_back(cluster);
    }
}

void MergeSCAN(vector<pair<double, double>>& intervals, double eps, ll m, vector<set<int>>& clusters){
    if(intervals.empty()) return;

    vector<pair<pair<double, double>, int>> interval_id_list;
    for(int idx = 0;idx < intervals.size();idx++) interval_id_list.emplace_back(intervals[idx], idx);
    sort(interval_id_list.begin(), interval_id_list.end(), [](const auto& interval_id1, const auto& interval_id2){
        return interval_id1.first < interval_id2.first;
    });
    vector<int> id_list;
    for(auto & interval_id : interval_id_list) id_list.push_back(interval_id.second);

    int left_idx = 0, right_idx = 0;
    double border = interval_id_list.front().first.first + eps;
    while(right_idx < interval_id_list.size() - 1){
        int next_idx = right_idx + 1;
        double next_start = interval_id_list[next_idx].first.first;
        if(next_start <= border) right_idx++;
        else{
            if(right_idx - left_idx + 1 >= m)
                clusters.emplace_back(id_list.begin() + left_idx, id_list.begin() + right_idx + 1);
            while(left_idx != next_idx){
                pair<double, double>& left_interval = interval_id_list[left_idx].first;
                if(next_start - left_interval.first <= eps) break;
                else left_idx++;
            }
            border = interval_id_list[left_idx].first.first + eps;
            right_idx++;
        }
    }
    if(right_idx - left_idx + 1 >= m)
        clusters.emplace_back(id_list.begin() + left_idx, id_list.begin() + right_idx + 1);
}

#endif //PROJECT_GROUP_H

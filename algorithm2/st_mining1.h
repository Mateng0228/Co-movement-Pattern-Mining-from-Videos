#ifndef PROJECT_ST_MINING1_H
#define PROJECT_ST_MINING1_H

#include <algorithm>
#include <set>
#include "tree.h"
#include "../result.h"

typedef long long ll;

class ST_Miner1{
private:
    ll camera_length;
    int car_length;
    vector<ll> &begin_ids;
    vector<int> &cars;
    MiningTree &mining_tree;

    struct Group{
        set<int> cluster;
        int begin_time, end_time;
        Group(set<int> &cluster, int begin, int end){
            this->cluster = cluster;
            begin_time = begin;
            end_time = end;
        }
        int life_time() const{
            return end_time - begin_time + 1;
        }
        bool operator<(const Group &group2) const{
            if(begin_time == group2.begin_time){
                if(end_time == group2.end_time) return cluster < group2.cluster;
                return end_time < group2.end_time;
            }
            return begin_time < group2.begin_time;
        }
    };
    void get_clusters(int camera_idx, ll m, double eps, vector<set<int>> &clusters);
    void add_result(Result &result, Group &candidate, ll m, ll k);
    void collect_items(map<int, vector<int>>&, map<int, vector<int>>::iterator, vector<int>&, vector<vector<int>>&);
public:
    ST_Miner1(ll length, vector<ll> &begin_ids, vector<int> &cars, MiningTree &tree) : camera_length(length), begin_ids(begin_ids), cars(cars), mining_tree(tree){
        car_length = cars.size();
    }

    void mine(Result &result, ll m, ll k, double eps);
};

void ST_Miner1::mine(Result &result, ll m, ll k, double eps) {
    vector<Group> candidates;
    vector<set<int>> clusters_first;
    get_clusters(0, m, eps, clusters_first);
    for(set<int> &cluster : clusters_first) candidates.emplace_back(cluster, 0, 0);

    for(int camera_idx = 1; camera_idx < camera_length; camera_idx++){
        vector<set<int>> clusters;
        get_clusters(camera_idx, m, eps, clusters);

        set<Group> next_candidates;
        for(Group& candidate : candidates){
            bool assigned = false;
            for(set<int>& cluster : clusters){
                set<int> intersection;
                set_intersection(candidate.cluster.begin(),candidate.cluster.end(),cluster.begin(),cluster.end(),inserter(intersection,intersection.begin()));

                if(intersection.size() < m) continue;
                else{
                    if(intersection.size() == candidate.cluster.size()) assigned = true;
                    next_candidates.insert(Group(intersection, 0, camera_idx));
                }
            }
            if(candidate.life_time() >= k){
                if(result.redundant_flag) add_result(result, candidate, m, k);
                else{
                    if(!assigned) add_result(result, candidate, m, k);
                }
            }
//            if(!assigned && candidate.life_time() >= k){
//                add_result(result, candidate, m, k);
//            }
        }
        candidates = vector<Group>(next_candidates.begin(), next_candidates.end());
    }
    for(Group &candidate : candidates){
        if(candidate.life_time() >= k){
            add_result(result, candidate, m, k);
        }
    }
}

void ST_Miner1::get_clusters(int camera_idx, ll m, double eps, vector<set<int>> &clusters){
    vector<pair<double, int>> info_list;
    vector<int> id_list;

    position* text = mining_tree.text;
    for(int car_idx = 0; car_idx < car_length; car_idx++){
        ll text_idx = begin_ids[car_idx] + camera_idx;
        double interval_left = text[text_idx].interval_left;
        info_list.emplace_back(interval_left, car_idx);
    }
    sort(info_list.begin(), info_list.end(), [](const auto &info1, const auto &info2){
        return info1.first < info2.first;
    });
    for(pair<double, int> &p : info_list) id_list.push_back(p.second);

    int left_idx = 0, right_idx = 0;
    double border = info_list.front().first + eps;
    while(right_idx < info_list.size() - 1){
        int next_idx = right_idx + 1;
        double next_start = info_list[next_idx].first;
        if(next_start <= border) right_idx++;
        else{
            if(right_idx - left_idx + 1 >= m) clusters.emplace_back(id_list.begin() + left_idx, id_list.begin() + right_idx + 1);
            while(left_idx != next_idx){
                if(next_start - info_list[left_idx].first <= eps) break;
                else left_idx++;
            }
            border = info_list[left_idx].first + eps;
            right_idx++;
        }
    }
    if(right_idx - left_idx + 1 >= m) clusters.emplace_back(id_list.begin() + left_idx, id_list.begin() + right_idx + 1);
}

void ST_Miner1::add_result(Result &result, Group &candidate, ll m, ll k) {
    bool need_collect = false;
    map<int, vector<int>> car2ids;
    for(int car_id : candidate.cluster){
        int car = cars[car_id];
        auto it  =car2ids.find(car);
        if(it == car2ids.end()) car2ids[car] = vector<int>{car_id};
        else{
            need_collect = true;
            it->second.push_back(car_id);
        }
    }
    if(car2ids.size() < m) return;

    vector<vector<int>> car_ids_list;
    if(need_collect){
        vector<int> car_ids;
        collect_items(car2ids, car2ids.begin(), car_ids, car_ids_list);
    }
    else{
        vector<int> car_ids;
        for(auto& entry : car2ids) car_ids.push_back(entry.second.front());
        car_ids_list.push_back(car_ids);
    }

    vector<ll> key;
    for(auto &entry : car2ids) key.push_back(entry.first);
    vector<vector<ll>> value;
    for(vector<int> &car_ids : car_ids_list){
        value.emplace_back();
        vector<ll> &temp_ids = value.back();
        for(int car_id : car_ids) temp_ids.push_back(begin_ids[car_id] + candidate.begin_time);
    }
    result.insert(key, value, candidate.end_time - candidate.begin_time + 1);
}

void ST_Miner1::collect_items(map<int, vector<int>>& car2ids, map<int, vector<int>>::iterator it, vector<int>& car_ids, vector<vector<int>>& car_ids_list) {
    if(it == car2ids.end()){
        car_ids_list.push_back(car_ids);
        return;
    }
    for(int car_id : it->second){
        car_ids.push_back(car_id);
        collect_items(car2ids, next(it), car_ids, car_ids_list);
        car_ids.pop_back();
    }
}

//预备get_clusters方法，问题是一个position所属的clusters有点多
//void ST_Miner1::get_clusters(int camera_idx, ll m, double eps, vector<set<int>>& clusters) {
//    int n_clusters = 0;
//    vector<vector<int>> clusters_list;
//    vector<double> begin_times, end_times;
//    map<int, int> id2pos;
//
//    position* text = mining_tree.text;
//    for(int car_idx = 0; car_idx < car_length; car_idx++){
//        ll text_idx = begin_ids[car_idx] + camera_idx;
//        position &p = text[text_idx];
//        double interval_left = p.interval_left;
//        vector<int>& contain_clusters = p.clusters;
//        for(int cluster_id : contain_clusters){
//            auto it = id2pos.find(cluster_id);
//            if(it == id2pos.end()){
//                id2pos[cluster_id] = n_clusters;
//                clusters_list.push_back(vector<int>{car_idx});
//                begin_times.push_back(interval_left);
//                end_times.push_back(interval_left);
//                n_clusters += 1;
//            }
//            else{
//                int cluster_pos = it->second;
//                clusters_list[cluster_pos].push_back(car_idx);
//                if(begin_times[cluster_pos] > interval_left) begin_times[cluster_pos] = interval_left;
//                if(end_times[cluster_pos] < interval_left) end_times[cluster_pos] = interval_left;
//            }
//        }
//    }
//
//    vector<int> exact_pos_list;
//    double prev_begin = -1, prev_end = -1;
//    for(auto &entry : id2pos){
//        int cluster_pos = entry.second;
//        if(clusters_list[cluster_pos].size() < m) continue;
//        double curr_begin = begin_times[cluster_pos], curr_end = end_times[cluster_pos];
//        if(exact_pos_list.empty()){
//            exact_pos_list.push_back(cluster_pos);
//            prev_begin = curr_begin;
//            prev_end = curr_end;
//            continue;
//        }
//        if(prev_end == curr_end) continue;
//        if(prev_begin == curr_begin){
//            exact_pos_list.back() = cluster_pos;
//            prev_end = curr_end;
//        }
//        else{
//            exact_pos_list.push_back(cluster_pos);
//            prev_begin = curr_begin;
//            prev_end = curr_end;
//        }
//    }
//
//    for(int cluster_pos : exact_pos_list){
//        vector<int> &cluster = clusters_list[cluster_pos];
//        clusters.emplace_back(cluster.begin(), cluster.end());
//    }
//}

#endif //PROJECT_ST_MINING1_H

#ifndef PROJECT_MINING_BASE_H
#define PROJECT_MINING_BASE_H

#include <algorithm>
#include <set>
#include "tree.h"
#include "result.h"
#include "../utils.h"

typedef long long ll;

class Base_Miner{
private:
    ll camera_length;
    int car_length;
    vector<ll> &begin_ids;
    vector<int> &cars;
    MiningTree &mining_tree;

    void get_clusters(int camera_idx, ll m, double eps, vector<pair<int, int>> &car2position, vector<pair<int, int>> &clusters);
    void add_result(Result &result, vector<int> &cluster, int begin_time, int end_time, ll m);
    void collect_items(map<int, vector<int>>&, map<int, vector<int>>::iterator, vector<int>&, vector<vector<int>>&);
public:
    Base_Miner(ll length, vector<ll> &begin_ids, vector<int> &cars, MiningTree &tree) : camera_length(length), begin_ids(begin_ids), cars(cars), mining_tree(tree){
        car_length = cars.size();
    }

    void mine(Result &result, ll m, ll k, double eps);
};

void Base_Miner::mine(Result &result, ll m, ll k, double eps) {
    // 待延长的候选convoys
    vector<vector<int>> pre_convoys;
    vector<int> begin_times;
    // pre_convoys的倒排表
    vector<pair<int, vector<int>>> inversion_lst;

    pre_convoys.emplace_back();
    begin_times.push_back(0);
    vector<int> &dummy_convoy = pre_convoys.front();
    for(int car_id = 0; car_id < car_length; car_id++){
        dummy_convoy.push_back(car_id);
        inversion_lst.emplace_back(car_id, vector<int>{0});
    }

    for(int camera_idx = 0; camera_idx < camera_length; camera_idx++){
        vector<pair<int, int>> car2position; // 记录car_id在倒排表中的位置
        for(int invert_id = 0; invert_id < inversion_lst.size(); invert_id++)
            car2position.emplace_back(inversion_lst[invert_id].first, invert_id);
        if(car2position.empty()) break;
        // 获得聚类
        vector<pair<int, int>> clusters;
        get_clusters(camera_idx, m, eps, car2position, clusters);
        // 初始化辅助数据结构
        set<pair<vector<int>, int>> candidates; // pair(car_ids, begin_time)
        vector<bool> extend_flags;
        vector<int> in_checks;
        for(int convoy_id = 0; convoy_id < pre_convoys.size(); convoy_id++){
            extend_flags.push_back(false);
            in_checks.push_back(-1);
        }
        // 延长pre_convoys到当前摄像头
        for(int cluster_idx = 0; cluster_idx < clusters.size(); cluster_idx++){
            pair<int, int> &cluster_bound = clusters[cluster_idx];
            vector<vector<int>> intersections;
            for(int convoy_id = 0; convoy_id < pre_convoys.size(); convoy_id++) intersections.emplace_back();
            set<int> check_st;

            for(int idx = cluster_bound.first; idx <= cluster_bound.second; idx++){
                int car_id = car2position[idx].first;
                int pre_position = car2position[idx].second;
                vector<int> &belong_convoys = inversion_lst[pre_position].second;
                for(int belong_convoy : belong_convoys){
                    intersections[belong_convoy].push_back(car_id);
                    if(intersections[belong_convoy].size() >= m && in_checks[belong_convoy] != cluster_idx){
                        in_checks[belong_convoy] = cluster_idx;
                        check_st.insert(belong_convoy);
                    }
                }
            }
            // 收集符合条件的candidate
//            bool as_convoy = true;
            for(int check_convoy_id : check_st){
                vector<int> &intersection = intersections[check_convoy_id];
                candidates.insert(make_pair(vector<int>(intersection.begin(), intersection.end()), begin_times[check_convoy_id]));
//                if(intersection.size() == cluster_bound.second - cluster_bound.first + 1) as_convoy = false;
                if(intersection.size() == pre_convoys[check_convoy_id].size()) extend_flags[check_convoy_id] = true;
            }
//            if(as_convoy && camera_length - camera_idx >= k){
//                vector<int> crt_cluster;
//                for(int idx = cluster_bound.first; idx <= cluster_bound.second; idx++)
//                    crt_cluster.push_back(car2position[idx].first);
//                candidates.insert(make_pair(crt_cluster, camera_idx));
//            }
        }
        // 将无法继续延长的pre_convoy添加到待去重的结果集中
        for(int convoy_id = 0; convoy_id < pre_convoys.size(); convoy_id++){
            if(camera_idx - begin_times[convoy_id] < k) continue;
            if(result.redundant_flag)
                add_result(result, pre_convoys[convoy_id], begin_times[convoy_id], camera_idx - 1, m);
            else{
                if(!extend_flags[convoy_id])
                    add_result(result, pre_convoys[convoy_id], begin_times[convoy_id], camera_idx - 1, m);
            }
        }
        deduplicate(candidates);
        // 更新pre_convoys
        pre_convoys.clear();
        begin_times.clear();
        for(const auto &candidate : candidates){
            pre_convoys.emplace_back(candidate.first.begin(), candidate.first.end());
            begin_times.push_back(candidate.second);
        }
        // 更新invert_lst
        inversion_lst.clear();
        map<int, int> invert_position_map;
        for(pair<int, int> &item : car2position){
            int car_id = item.first;
            invert_position_map[car_id] = inversion_lst.size();
            inversion_lst.emplace_back(car_id, vector<int>());
        }
        for(int convoy_id = 0; convoy_id < pre_convoys.size(); convoy_id++){
            vector<int> &convoy = pre_convoys[convoy_id];
            for(int car_id : convoy){
                inversion_lst[invert_position_map[car_id]].second.push_back(convoy_id);
            }
        }
        for(auto it = inversion_lst.begin(); it != inversion_lst.end();){
            if(it->second.empty()) it = inversion_lst.erase(it);
            else it++;
        }
    }
    // 收集convoy到结果集
    for(int convoy_id = 0; convoy_id < pre_convoys.size(); convoy_id++){
        if(camera_length - begin_times[convoy_id] < k) continue;
        add_result(result, pre_convoys[convoy_id], begin_times[convoy_id], camera_length - 1, m);
    }
}

void Base_Miner::get_clusters(int camera_idx, ll m, double eps, vector<pair<int, int>> &car2position, vector<pair<int, int>> &clusters){
    position* text = mining_tree.text;
    sort(car2position.begin(), car2position.end(), [&](const pair<int, int> &item1, const pair<int, int> &item2){
        int car_id1 = item1.first, car_id2 = item2.first;
        double time1 = text[begin_ids[car_id1] + camera_idx].interval_left;
        double time2 = text[begin_ids[car_id2] + camera_idx].interval_left;
        return time1 < time2;
    });

    vector<double> time_lst;
    for(pair<int, int> &item : car2position){
        double curr_time = text[begin_ids[item.first] + camera_idx].interval_left;
        time_lst.push_back(curr_time);
    }
    int left_idx = 0, right_idx = 0;
    double border = time_lst.front() + eps;
    while(right_idx < car2position.size() - 1){
        int next_idx = right_idx + 1;
        double next_start = time_lst[next_idx];
        if(next_start <= border) right_idx++;
        else{
            if(right_idx - left_idx + 1 >= m) clusters.emplace_back(left_idx, right_idx);
            while(left_idx != next_idx){
                if(next_start - time_lst[left_idx] <= eps) break;
                else left_idx++;
            }
            border = time_lst[left_idx] + eps;
            right_idx++;
        }
    }
    if(right_idx - left_idx + 1 >= m) clusters.emplace_back(left_idx, right_idx);
}

void Base_Miner::add_result(Result &result, vector<int> &cluster, int begin_time, int end_time, ll m) {
    bool need_collect = false;
    map<int, vector<int>> car2ids;
    for(int car_id : cluster){
        int car = cars[car_id];
        auto it  = car2ids.find(car);
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
        for(int car_id : car_ids) temp_ids.push_back(begin_ids[car_id] + begin_time);
    }
    result.insert(key, value, end_time - begin_time + 1);
}

void Base_Miner::collect_items(map<int, vector<int>>& car2ids, map<int, vector<int>>::iterator it, vector<int>& car_ids, vector<vector<int>>& car_ids_list) {
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

#endif //PROJECT_MINING_BASE_H

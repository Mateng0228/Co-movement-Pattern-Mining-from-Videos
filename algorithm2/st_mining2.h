#ifndef PROJECT_ST_MINING2_H
#define PROJECT_ST_MINING2_H

#include <algorithm>
#include "tree.h"
#include "../result.h"

typedef long long ll;
class ST_Miner2{
private:
    ll camera_length;
    int car_length;
    vector<ll>& begin_ids;
    vector<int>& cars;
    MiningTree& mining_tree;

    void get_clusters(int camera_idx, ll m, double eps, vector<pair<int, int>> &car_ids, vector<pair<int, int>> &cluster_marks);
    void add_result(Result &result, vector<int> &cluster, int end_time, ll m);
    void collect_items(map<int, vector<int>>&, map<int, vector<int>>::iterator, vector<int>&, vector<vector<int>>&);
    struct Group{
        int max_size, offset = 0, top_cluster = -1;
        vector<int> items;
        list<pair<int, int>> delimiter;// (cluster_id, position in cars)
        bool extend_flag = false, contain_flag = false;
    };
public:
    ST_Miner2(ll length, vector<ll>& begin_ids, vector<int>& cars, MiningTree& tree):
        camera_length(length), begin_ids(begin_ids), cars(cars), mining_tree(tree){
        car_length = cars.size();
    }

    void mine(Result &result, ll m, ll k, double eps);
};

void ST_Miner2::mine(Result &result, ll m, ll k, double eps) {
    vector<vector<int>> pre_clusters;// 上次处理得到的聚类
    vector<pair<int, vector<int>>> pre_marks;// pre_clusters的倒排表(以car为键)

    pre_clusters.emplace_back();
    vector<int> &cluster = pre_clusters.front();
    for(int car_id = 0;car_id < car_length;car_id++){
        cluster.push_back(car_id);
        pre_marks.emplace_back(car_id, vector<int>{0});
    }
    for(int camera_idx = 0;camera_idx < camera_length;camera_idx++){
        vector<pair<int, int>> car_pos_list, curr_marks;
        for(int mark_id = 0;mark_id < pre_marks.size();mark_id++) car_pos_list.emplace_back(pre_marks[mark_id].first, mark_id);
        if(car_pos_list.empty()) break;

        get_clusters(camera_idx, m, eps, car_pos_list, curr_marks);

        map<int, Group> cluster2group;
        for(int cluster_id = 0;cluster_id < pre_clusters.size();cluster_id++){
            Group group;
            group.max_size = pre_clusters[cluster_id].size();
            cluster2group[cluster_id] = group;
        }

        // 连接聚类
        set<vector<int>> candidates;
        int handle_cluster = -1;
        for(int handle_id = 0;handle_id < curr_marks.size();handle_id++){
            int first_cluster = curr_marks[handle_id].first;
            if(first_cluster == -1) continue;

            if(handle_cluster == -1) handle_cluster = first_cluster;
            if(handle_cluster == first_cluster){
                int end_cluster = curr_marks[handle_id].second;

                int pre_pos = car_pos_list[handle_id].second;
                vector<int> &belong_clusters = pre_marks[pre_pos].second;
                for(int belong_cluster : belong_clusters){
                    Group &group = cluster2group[belong_cluster];
                    group.items.push_back(handle_id);
                    if(group.contain_flag) group.contain_flag = false;

                    if(group.top_cluster == -1) group.top_cluster = end_cluster;
                    if(group.top_cluster != end_cluster){
                        group.delimiter.emplace_back(group.top_cluster, group.items.size() - 2);
                        group.top_cluster = end_cluster;
                    }
                }
            }
            else{
                for(auto &entry : cluster2group){
                    Group &group = entry.second;
                    if(group.delimiter.empty()){
                        if(group.top_cluster == handle_cluster){
                            if(!group.contain_flag){
                                candidates.insert(vector<int>(group.items.begin() + group.offset, group.items.end()));
                                if(group.items.size() - group.offset == group.max_size) group.extend_flag = true;
                            }

                            group.offset = group.items.size();
                            group.top_cluster = -1;
                            group.contain_flag = false;
                        }
                    }
                    else{
                        int bottom_cluster = group.delimiter.front().first;
                        if(bottom_cluster == handle_cluster){
                            if(!group.contain_flag){
                                candidates.insert(vector<int>(group.items.begin() + group.offset, group.items.end()));
                                group.contain_flag = true;
                                if(group.items.size() - group.offset == group.max_size) group.extend_flag = true;
                            }

                            group.offset = group.delimiter.front().second + 1;
                            group.delimiter.pop_front();
                        }
                    }
                }
                handle_cluster = first_cluster;
                handle_id--;
            }
        }
        for(auto &entry : cluster2group){
            Group &group = entry.second;
            if(!group.contain_flag){
                candidates.insert(vector<int>(group.items.begin() + group.offset, group.items.end()));
                if(group.items.size() - group.offset == group.max_size) group.extend_flag = true;
            }
        }
        // 添加convoy到结果集
        if(camera_idx >= k){
            for(auto &entry : cluster2group){
                int cluster_id = entry.first;
                if(result.redundant_flag) add_result(result, pre_clusters[cluster_id], camera_idx - 1, m);
                else{
                    if(!entry.second.extend_flag)
                        add_result(result, pre_clusters[cluster_id], camera_idx - 1, m);
                }
            }
        }
        // 更新pre_clusters
        pre_clusters.clear();
        for(const vector<int> &candidate : candidates){
            pre_clusters.emplace_back();
            vector<int> &pre_cluster = pre_clusters.back();
            for(int handle_id : candidate) pre_cluster.push_back(car_pos_list[handle_id].first);
        }
        // 更新pre_marks
        pre_marks.clear();
        map<int, int> car2pos;
        for(pair<int, int> &p : car_pos_list){
            int car_id = p.first;
            car2pos[car_id] = pre_marks.size();
            pre_marks.emplace_back(car_id, vector<int>());
        }
        for(int cluster_id = 0;cluster_id < pre_clusters.size();cluster_id++){
            vector<int> &pre_cluster = pre_clusters[cluster_id];
            for(int car_id : pre_cluster){
                pre_marks[car2pos[car_id]].second.push_back(cluster_id);
            }
        }
        for(auto it = pre_marks.begin();it != pre_marks.end();){
            if(it->second.empty()) it = pre_marks.erase(it);
            else it++;
        }
    }
    // 收集convoy到结果集
    if(camera_length >= k){
        for(vector<int> &pre_cluster : pre_clusters){
            // add-results 0, camera_length
            add_result(result, pre_cluster, camera_length - 1, m);
        }
    }
}

void ST_Miner2::get_clusters(int camera_idx, ll m, double eps, vector<pair<int, int>> &car_ids, vector<pair<int, int>> &cluster_marks) {
    position* text = mining_tree.text;
    sort(car_ids.begin(), car_ids.end(), [&](const pair<int, int> &item1, const pair<int, int> &item2){
        int car_id1 = item1.first, car_id2 = item2.first;
        double time1 = text[begin_ids[car_id1] + camera_idx].interval_left;
        double time2 = text[begin_ids[car_id2] + camera_idx].interval_left;
        return time1 < time2;
    });

    // 获得聚类信息
    vector<pair<int, int>> clusters;
    vector<double> time_list;
    for(auto &p : car_ids){
        double curr_time = text[begin_ids[p.first] + camera_idx].interval_left;
        time_list.push_back(curr_time);
    }
    int left_idx = 0, right_idx = 0;
    double border = time_list.front() + eps;
    while(right_idx < car_ids.size() - 1){
        int next_idx = right_idx + 1;
        double next_start = time_list[next_idx];
        if(next_start <= border) right_idx++;
        else{
            if(right_idx - left_idx + 1 >= m) clusters.emplace_back(left_idx, right_idx);// 闭区间
            while(left_idx != next_idx){
                if(next_start - time_list[left_idx] <= eps) break;
                else left_idx++;
            }
            border = time_list[left_idx] + eps;
            right_idx++;
        }
    }
    if(right_idx - left_idx + 1 >= m) clusters.emplace_back(left_idx, right_idx);

    // 获得每个车辆的所属聚类信息
    for(auto &p : car_ids) cluster_marks.emplace_back(-1, -1);
    if(clusters.empty()) return;

    int pre_idx = clusters.front().first;
    for(int cluster_idx = 0;cluster_idx < clusters.size();cluster_idx++){
        int curr_idx = clusters[cluster_idx].second;
        for(int idx = pre_idx;idx <= curr_idx;idx++) cluster_marks[idx].first = cluster_idx;
        if(cluster_idx + 1 < clusters.size() && clusters[cluster_idx + 1].first > curr_idx) pre_idx = clusters[cluster_idx + 1].first;
        else pre_idx = curr_idx + 1;
    }
    pre_idx = clusters.back().second;
    for(int cluster_idx = clusters.size() - 1;cluster_idx >= 0;cluster_idx--){
        int curr_idx = clusters[cluster_idx].first;
        for(int idx = curr_idx;idx <= pre_idx;idx++) cluster_marks[idx].second = cluster_idx;
        if(cluster_idx - 1 >= 0 && clusters[cluster_idx - 1].second < curr_idx) pre_idx = clusters[cluster_idx - 1].second;
        else pre_idx = curr_idx - 1;
    }
}

void ST_Miner2::add_result(Result &result, vector<int> &cluster, int end_time, ll m) {
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
        for(int car_id : car_ids) temp_ids.push_back(begin_ids[car_id]);
    }
    result.insert(key, value, end_time + 1);
}

void ST_Miner2::collect_items(map<int, vector<int>>& car2ids, map<int, vector<int>>::iterator it, vector<int>& car_ids, vector<vector<int>>& car_ids_list) {
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

#endif //PROJECT_ST_MINING2_H

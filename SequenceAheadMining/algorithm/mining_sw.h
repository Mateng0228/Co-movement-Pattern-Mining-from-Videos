#ifndef PROJECT_MINING_SW_H
#define PROJECT_MINING_SW_H

#include <algorithm>
#include <deque>
#include "tree.h"
#include "result.h"

typedef long long ll;
class SW_Miner{
private:
    ll camera_length;
    int car_length;
    vector<ll> &begin_ids;
    vector<int> &cars;
    MiningTree &mining_tree;

    void get_clusters(int camera_idx, ll m, double eps, vector<pair<int, int>> &car_ids, vector<pair<int, int>> &cluster_marks);
    void add_result(Result &result, vector<int> &cluster, int begin_time, int end_time, ll m);
    void collect_items(map<int, vector<int>>&, map<int, vector<int>>::iterator, vector<int>&, vector<vector<int>>&);
    struct Group{
        int max_size = 0;
        deque<int> items;
        bool extend_flag = false;
        bool contain_flag = false;
        bool in_check = false;
    };
public:
    SW_Miner(ll length, vector<ll> &begin_ids, vector<int> &cars, MiningTree &tree):
            camera_length(length), begin_ids(begin_ids), cars(cars), mining_tree(tree){
        car_length = cars.size();
    }

    void mine(Result &result, ll m, ll k, double eps);
};

void SW_Miner::mine(Result &result, ll m, ll k, double eps) {
    // 待延长的候选convoys
    vector<vector<int>> pre_convoys;
    vector<int> begin_times;
    // pre_convoys的倒排表
    vector<pair<int, vector<int>>> inversion_lst;

    // 初始化dummy convoy
    pre_convoys.emplace_back();
    begin_times.push_back(0);
    vector<int> &dummy_convoy = pre_convoys.front();
    for(int car_id = 0;car_id < car_length;car_id++){
        dummy_convoy.push_back(car_id);
        inversion_lst.emplace_back(car_id, vector<int>{0});
    }
    // 开始逐个摄像头的延长流程
    for(int camera_idx = 0; camera_idx < camera_length; camera_idx++){
        vector<pair<int, int>> car2position; // 记录car_id在倒排表中的位置
        for(int inversion_id = 0; inversion_id < inversion_lst.size(); inversion_id++)
            car2position.emplace_back(inversion_lst[inversion_id].first, inversion_id);
        if(car2position.empty()) break;

        vector<pair<int, int>> belong_clusters;
        get_clusters(camera_idx, m, eps, car2position, belong_clusters);

        deque<int> window; // 滑动窗口
        vector<Group> groups;
        for(vector<int> &pre_convoy : pre_convoys){
            groups.emplace_back();
            Group &group = groups.back();
            group.max_size = pre_convoy.size();
        }
        set<int> check_lst; // 用于标识那些交集中元素个数大于等于m的待延长convoys

        // 连接聚类
        set<pair<vector<int>, int>> candidates; // pair(car_ids, begin_time)
        int handle_cluster = -1;
        for(int handle_id = 0; handle_id < belong_clusters.size(); handle_id++){
            int first_cluster = belong_clusters[handle_id].first;
            if(first_cluster == -1) continue;

            if(handle_cluster == -1) handle_cluster = first_cluster;
            if(handle_cluster == first_cluster){
                window.push_back(handle_id);
                int pre_pos = car2position[handle_id].second;
                for(int convoy_id : inversion_lst[pre_pos].second){
                    Group &group = groups[convoy_id];
                    group.items.push_back(handle_id);
                    if(group.contain_flag) group.contain_flag = false;
                    if(group.items.size() >= m && !group.in_check){
                        group.in_check = true;
                        check_lst.insert(convoy_id);
                    }
                }
            }
            else{
                // 收集符合条件的candidate
//                bool as_convoy = true;
                for(int convoy_id : check_lst){
                    Group &group = groups[convoy_id];
                    if(group.contain_flag) continue;

                    deque<int> &items = group.items;
                    int min_top_cluster = belong_clusters[items.front()].second;
                    if(min_top_cluster == handle_cluster){
                        candidates.insert(make_pair(vector<int>(items.begin(), items.end()), begin_times[convoy_id]));
//                        if(items.size() == window.size()) as_convoy = false;
                        if(items.size() == group.max_size) group.extend_flag = true;
                        group.contain_flag = true;
                    }
                }
//                if(as_convoy && camera_length - camera_idx >= k)
//                    candidates.insert(make_pair(vector<int>(window.begin(), window.end()), camera_idx));
                // 删除不可能属于之后要处理的聚类的车辆
                while(!window.empty()){
                    int crt_handle_id = window.front();
                    if(belong_clusters[crt_handle_id].second != handle_cluster) break;

                    window.pop_front();
                    int pre_pos = car2position[crt_handle_id].second;
                    for(int convoy_id : inversion_lst[pre_pos].second){
                        Group &group = groups[convoy_id];
                        group.items.pop_front();
                        if(group.in_check && group.items.size() < m){
                            group.in_check = false;
                            check_lst.erase(convoy_id);
                        }
                    }
                }
                handle_cluster = first_cluster;
                handle_id--;
            }
        }
        // 收集符合条件的candidate
//        bool as_convoy = true;
        for(int convoy_id : check_lst){
            Group &group = groups[convoy_id];
            if(group.contain_flag) continue;

            deque<int> &items = group.items;
            candidates.insert(make_pair(vector<int>(items.begin(), items.end()), begin_times[convoy_id]));
//            if(items.size() == window.size()) as_convoy = false;
            if(items.size() == group.max_size) group.extend_flag = true;
        }
//        if(as_convoy && camera_length - camera_idx >= k)
//            candidates.insert(make_pair(vector<int>(window.begin(), window.end()), camera_idx));
        // 添加convoy到结果集
        for(int convoy_id = 0; convoy_id < pre_convoys.size(); convoy_id++){
            if(camera_idx - begin_times[convoy_id] < k) continue;
            if(result.redundant_flag)
                add_result(result, pre_convoys[convoy_id], begin_times[convoy_id], camera_idx - 1, m);
            else{
                if(!groups[convoy_id].extend_flag)
                    add_result(result, pre_convoys[convoy_id], begin_times[convoy_id], camera_idx - 1, m);
            }
        }
        // 更新pre_convoys和begin_times
        pre_convoys.clear();
        begin_times.clear();
        for(const auto &p : candidates){
            pre_convoys.emplace_back();
            vector<int> &pre_convoy = pre_convoys.back();
            for(int handle_id : p.first) pre_convoy.push_back(car2position[handle_id].first);
            begin_times.push_back(p.second);
        }
        // 更新inversion_lst
        inversion_lst.clear();
        map<int, int> car2pos;
        for(pair<int, int> &p : car2position){
            int car_id = p.first;
            car2pos[car_id] = inversion_lst.size();
            inversion_lst.emplace_back(car_id, vector<int>());
        }
        for(int convoy_id = 0; convoy_id < pre_convoys.size(); convoy_id++){
            vector<int> &pre_convoy = pre_convoys[convoy_id];
            for(int car_id : pre_convoy) inversion_lst[car2pos[car_id]].second.push_back(convoy_id);
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

void SW_Miner::get_clusters(int camera_idx, ll m, double eps, vector<pair<int, int>> &car_ids, vector<pair<int, int>> &cluster_marks) {
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
//double insert_time = 0.0;
void SW_Miner::add_result(Result &result, vector<int> &cluster, int begin_time, int end_time, ll m) {
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
//    clock_t insert_before = clock();
    result.insert(key, value, end_time - begin_time + 1);
//    insert_time += static_cast<double>(clock() - insert_before);
}

void SW_Miner::collect_items(map<int, vector<int>>& car2ids, map<int, vector<int>>::iterator it, vector<int>& car_ids, vector<vector<int>>& car_ids_list) {
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

#endif //PROJECT_MINING_SW_H

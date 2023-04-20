#ifndef PROJECT_CLUSTER_H
#define PROJECT_CLUSTER_H

#include <map>
#include <algorithm>
#include <iostream>
#include "position.h"

#define UNCERTAIN (-1)
using namespace std;
typedef long long ll;

struct car_info{
    double interval_left, interval_right;
    int object;
    int position_idx;
    car_info(double left, double right, int object, int position_idx){
        this->interval_left = left;
        this->interval_right = right;
        this->object = object;
        this->position_idx = position_idx;
    }
};

void temporal_cluster(vector<car_info>& car_infos, vector<pair<int, int>>& clusters, vector<pair<int, int>>& groups, ll m, double eps){
    if(car_infos.empty()) return;

    sort(car_infos.begin(), car_infos.end(), [](const car_info &info1, const car_info &info2){
        if(info1.interval_left == info2.interval_left) return info1.interval_right < info2.interval_right;
        return info1.interval_left < info2.interval_left;
    });

    int group_left_idx = 0, group_right_idx = 0;// 记录当前group的第一个聚类位置和最后一个聚类位置
    int left_idx = 0, right_idx = 0;// 记录当前聚类的第一个元素位置和最后一个元素位置
    int pre_right_idx = -1;// 记录上个聚类的最后一个元素所在位置
    double border = car_infos.front().interval_left + eps;
    while(right_idx != car_infos.size() - 1){
        int next_idx = right_idx + 1;
        double next_left = car_infos[next_idx].interval_left;
        if(next_left <= border) right_idx++;
        else{
            if(right_idx - left_idx + 1 >= m){
                // group相关
                if(pre_right_idx != -1){
                    if(pre_right_idx < left_idx){
                        groups.emplace_back(group_left_idx, group_right_idx);
                        group_left_idx = group_right_idx + 1;
                        group_right_idx = group_left_idx;
                    }
                    else group_right_idx++;
                }

                clusters.emplace_back(left_idx, right_idx);
                pre_right_idx = right_idx;
            }
            left_idx++;
            while(left_idx != next_idx){
                double temp_border = car_infos[left_idx].interval_left + eps;
                if(next_left <= temp_border) break;
                else left_idx++;
            }
            border = car_infos[left_idx].interval_left + eps;
            right_idx++;
        }
    }
    if(right_idx - left_idx + 1 >= m){
        // group相关
        if(pre_right_idx != -1){
            if(pre_right_idx < left_idx){
                groups.emplace_back(group_left_idx, group_right_idx);
                groups.emplace_back(group_right_idx + 1, group_right_idx + 1);
            }
            else groups.emplace_back(group_left_idx, group_right_idx + 1);
        }
        else groups.emplace_back(group_left_idx, group_right_idx);

        clusters.emplace_back(left_idx, right_idx);
        pre_right_idx = right_idx;
    }
    else if(pre_right_idx != -1) groups.emplace_back(group_left_idx, group_right_idx);
}

void add_cluster_mark(vector<position> *arr_positions, int arr_size, ll m, double eps){
    // 按摄像头组织数据
    map<ll, vector<car_info>> camera_map;
    for(int arr_idx = 0;arr_idx < arr_size;arr_idx++){
        vector<position>& positions = arr_positions[arr_idx];
        int camera_length = positions.size();
        for(int idx = 0; idx < camera_length; idx++){
            position& position = positions[idx];
            auto it = camera_map.find(position.camera);
            if(it == camera_map.end()){
                vector<car_info> car_infos;
                car_infos.emplace_back(position.interval_left, position.interval_right, arr_idx, idx);
                camera_map[position.camera] = car_infos;
            }
            else it->second.emplace_back(position.interval_left, position.interval_right, arr_idx, idx);
        }
    }
    // 逐个摄像头聚类并标号
    for(auto & entry : camera_map){
        ll camera = entry.first;
        vector<car_info>& car_infos = entry.second;
        vector<pair<int, int>> clusters;
        vector<pair<int, int>> groups;
        temporal_cluster(car_infos, clusters, groups, m, eps);

        for(int group_idx = 0;group_idx < groups.size();group_idx++){
            int begin_cluster_idx = groups[group_idx].first;
            int end_cluster_idx = groups[group_idx].second;
            for(int cluster_idx = begin_cluster_idx; cluster_idx <= end_cluster_idx; cluster_idx++){
                pair<int, int>& cluster_pair = clusters[cluster_idx];
                for(int item_idx = cluster_pair.first;item_idx <= cluster_pair.second;item_idx++){
                    car_info& info = car_infos[item_idx];
                    position& pos = arr_positions[info.object][info.position_idx];
                    pos.group = group_idx;
                    pos.clusters.push_back(cluster_idx);
                }
            }
        }
    }

}

void split_positions(vector<position> *init_positions, int init_size, ll k, vector<vector<position>>& positions_list, vector<int>& sub2init){
    for(int init_idx = 0;init_idx < init_size;init_idx++){
        vector<position>& positions = init_positions[init_idx];
        int left_idx = UNCERTAIN, right_idx = UNCERTAIN;
        for(int cur_idx = 0;cur_idx < positions.size();cur_idx++){
            int cur_group = positions[cur_idx].group;
            if(left_idx == UNCERTAIN){
                if(cur_group != -1){
                    left_idx = cur_idx;
                    right_idx = cur_idx;
                }
            }
            else{
                if(cur_group == -1){
                    if(right_idx - left_idx + 1 >= k){
                        positions_list.emplace_back(positions.begin() + left_idx, positions.begin() + right_idx + 1);
                        sub2init.push_back(init_idx);
                    }
                    left_idx = UNCERTAIN;
                    right_idx = UNCERTAIN;
                }
                else right_idx++;
            }
        }
        if(left_idx != UNCERTAIN && right_idx - left_idx + 1 >= k){
            positions_list.emplace_back(positions.begin() + left_idx, positions.begin() + right_idx + 1);
            sub2init.push_back(init_idx);
        }
    }
}

#endif //PROJECT_CLUSTER_H

#ifndef EXPERIMENT_BASELINE_EXT_H
#define EXPERIMENT_BASELINE_EXT_H

#include "structs.h"
#include "Offline/position.h"
#include "Offline/cluster.h"
#include "Offline/result.h"
#include "Offline/mining_sw.h"


class Baseline_Ext{
private:
    int m, k; double eps;
    vector<vector<offline_position>> paths;
    map<int, int> op; // object_id to path_id
    map<ll, vector<car_info>> camera_map;
    MiningTree *mining_tree = nullptr;
    OfflineResult result;

    void update_mark(ll camera);
    int update_simplified_paths(int target_path_id, vector<vector<offline_position>> &simplified_paths, vector<vector<int>> &pst_ids_lst, vector<int> &path_ids);
    bool query(int target_sp_id, ll node_id, vector<int> &path_ids);
    int count_unique(vector<int> &lst);
public:
    Baseline_Ext(int m, int k, double eps) : m(m), k(k), eps(eps), result(true, true){}
    ~Baseline_Ext() {delete mining_tree;}
    void update(int oid, Position position);
    void print(bool detail = false);
};

void Baseline_Ext::update(int oid, Position position){
    int path_id = -1;
    auto op_itr = op.find(oid);
    if(op_itr == op.end()){
        path_id = static_cast<int>(op.size());
        op.insert(make_pair(oid, path_id));
        paths.emplace_back();
    }
    else path_id = op_itr->second;
    paths[path_id].emplace_back(position.camera, position.ts, position.te);

    int position_id = static_cast<int>(paths[path_id].size()) - 1;
    camera_map[position.camera].emplace_back(position.ts, position.te, path_id, position_id);
    update_mark(position.camera);

    vector<vector<offline_position>> simplified_paths;
    vector<vector<int>> pst_ids_lst;
    vector<int> path_ids;
    int simplified_id = update_simplified_paths(path_id, simplified_paths, pst_ids_lst, path_ids);

    if(simplified_id == -1 || simplified_paths.empty()) return;
    delete mining_tree;
    mining_tree = new MiningTree(simplified_paths);
    result.update_pointers(mining_tree, &simplified_paths, &pst_ids_lst);
    query(simplified_id, mining_tree->root, path_ids);
    result.de_duplication();
}

void Baseline_Ext::print(bool detail){
    if(!detail) result.print_contents();
    else result.print_contents(*mining_tree);
}

void Baseline_Ext::update_mark(ll camera){
    vector<car_info> &car_infos = camera_map.at(camera);
    vector<pair<int, int>> clusters;
    vector<pair<int, int>> groups;
    temporal_cluster(car_infos, clusters, groups, m, eps);

    for(int group_idx = 0; group_idx < groups.size(); ++group_idx){
        int begin_cluster_idx = groups[group_idx].first;
        int end_cluster_idx = groups[group_idx].second;
        for(int cluster_idx = begin_cluster_idx; cluster_idx <= end_cluster_idx; ++cluster_idx){
            pair<int, int> &cluster_pair = clusters[cluster_idx];
            for(int item_idx = cluster_pair.first; item_idx <= cluster_pair.second; ++item_idx){
                car_info &info = car_infos[item_idx];
                offline_position &pos = paths[info.object][info.position_idx]; // 这里的info.object实际上指的是path_id
                pos.group = group_idx;
                pos.clusters.push_back(cluster_idx);
            }
        }
    }
}

int Baseline_Ext::update_simplified_paths(int target_path_id, vector<vector<offline_position>> &simplified_paths, vector<vector<int>> &pst_ids_lst, vector<int> &path_ids){
    int target_simplified_id = -1;
    for(int path_id = 0; path_id < paths.size(); ++path_id){
        vector<offline_position> &path = paths[path_id];
        int left_idx = UNCERTAIN, right_idx = UNCERTAIN;
        for(int pos_id = 0; pos_id < path.size(); ++pos_id){
            int cur_group = path[pos_id].group;
            if(left_idx == UNCERTAIN){
                if(cur_group != -1){
                    left_idx = pos_id;
                    right_idx = pos_id;
                }
            }
            else{
                if(cur_group != -1) right_idx++;
                else{
                    if(right_idx - left_idx + 1 >= k){
                        simplified_paths.emplace_back(path.begin() + left_idx, path.begin() + right_idx + 1);
                        vector<int> pst_ids;
                        for(int pst_id = left_idx; pst_id <= right_idx; ++pst_id) pst_ids.push_back(pst_id);
                        pst_ids_lst.push_back(pst_ids);
                        path_ids.push_back(path_id);
                    }
                    left_idx = UNCERTAIN; right_idx = UNCERTAIN;
                }
            }
        }
        if(left_idx != UNCERTAIN && right_idx - left_idx + 1 >= k){
            simplified_paths.emplace_back(path.begin() + left_idx, path.begin() + right_idx + 1);
            vector<int> pst_ids;
            for(int pst_id = left_idx; pst_id <= right_idx; ++pst_id) pst_ids.push_back(pst_id);
            pst_ids_lst.push_back(pst_ids);
            path_ids.push_back(path_id);

            if(path_id == target_path_id) target_simplified_id = static_cast<int>(path_ids.size()) - 1;
        }
    }
    return target_simplified_id;
}

bool Baseline_Ext::query(int target_sp_id, ll node_id, vector<int> &path_ids){
    bool contain_flag = false; // 是否包含更新的摄像头位置

    MiningNode *tree = mining_tree->tree;
    if(tree[node_id].is_leaf){
        int sp_id = mining_tree->terminator2ListId[mining_tree->text[tree[node_id].end - 1].camera];
        if(sp_id == target_sp_id && tree[node_id].edge_length() == 1) contain_flag = true;
        return contain_flag;
    }

    for(auto &entry : tree[node_id].next){
        ll nxt_id = entry.second;
        bool crt_flag = query(target_sp_id, nxt_id, path_ids);
        contain_flag = (contain_flag || crt_flag);
    }
    if(contain_flag){
        contain_flag = false;
        if(tree[node_id].depth >= k){
            ll common_length = tree[node_id].depth;
            ll leaf_left = tree[node_id].leaf_left, leaf_right = tree[node_id].leaf_right;
            if(leaf_right - leaf_left + 1 >= m){
                vector<ll> begin_tids;
                vector<int> cars;
                for(ll i = leaf_left; i <= leaf_right; ++i){
                    MiningNode &leaf_node = tree[mining_tree->leaves[i]];
                    ll begin_tid = leaf_node.end - leaf_node.depth;
                    begin_tids.push_back(begin_tid);
                    int sp_id = mining_tree->terminator2ListId[mining_tree->text[leaf_node.end - 1].camera];
                    cars.push_back(path_ids[sp_id]);
                }
                int n_cars = count_unique(cars);
                if(n_cars >= m){
                    SW_Miner miner(common_length, begin_tids, cars, *mining_tree);
                    miner.mine(result, m, k, eps);
                }
            }
        }
    }
    return contain_flag;
}

int Baseline_Ext::count_unique(vector<int> &lst) {
    if(lst.empty()) return 0;

    vector<int> temp_list = lst;
    sort(temp_list.begin(), temp_list.end());
    int counts = 1, pre_val = temp_list[0];
    for(int i = 1;i < temp_list.size();i++){
        if(temp_list[i] != pre_val){
            counts++;
            pre_val = temp_list[i];
        }
    }
    return counts;
}

#endif //EXPERIMENT_BASELINE_EXT_H

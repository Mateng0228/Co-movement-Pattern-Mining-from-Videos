#ifndef PROJECT_RESULT_H
#define PROJECT_RESULT_H

#include <vector>
#include <list>
#include <map>
#include <unordered_set>
#include <unordered_map>
#include <climits>
#include <iostream>
#include <fstream>
#include "tree.h"

using namespace std;
typedef long long ll;

class Result{
public:
    bool redundant_flag = false;//是否保存所有（意味着冗余）满足条件的结果

    virtual ~Result() = default;
    virtual void insert(vector<ll> &cars, vector<vector<ll>> &begin_ids_list, int length) = 0;
    virtual void de_duplication() = 0;
    virtual void print_contents() = 0;
};

class ResultBaseImpl : public Result{
private:
    map<vector<ll>, list<vector<pair<ll, ll>>>> result_map;

    // 去除两个键值对之间可能的冗余项，其中idx1指向的键包含idx2指向的键
    void de_duplication(vector<map<vector<ll>, list<vector<pair<ll, ll>>>>::iterator> &idx2itr, int idx1, int idx2){
        const vector<ll> &cars1 = idx2itr[idx1]->first, &cars2 = idx2itr[idx2]->first;
        vector<int> car_ids;// cars2包含的car分别在cars1中的位置
        int p1 = 0, p2 = 0;
        while(p2 != cars2.size()){
            if(p1 == cars1.size()){
                cerr<<"cars1 并不包含 cars2"<<endl;
                return;
            }
            if(cars1[p1] != cars2[p2]) p1++;
            else{
                car_ids.push_back(p1);
                p1++; p2++;
            }
        }
        // 开始逐个去重
        list<vector<pair<ll, ll>>> &intervals1 = idx2itr[idx1]->second, &intervals2 = idx2itr[idx2]->second;
        for(auto itr = intervals2.begin(); itr != intervals2.end();){
            vector<pair<ll, ll>> &target_ranges = *itr;
            bool delete_flag = false;
            for(vector<pair<ll, ll>> &total_ranges : intervals1){
                vector<pair<ll, ll>> current_ranges;
                for(int car_id : car_ids) current_ranges.push_back(total_ranges[car_id]);
                bool contain_flag = true;// 表示current_ranges是否完全包含target_ranges
                for(int range_id = 0; range_id < target_ranges.size(); range_id++){
                    pair<ll, ll> &target_range = target_ranges[range_id], &current_range = current_ranges[range_id];
                    if(!(current_range.first <= target_range.first && current_range.second >= target_range.second)){
                        contain_flag = false;
                        break;
                    }
                }
                if(contain_flag){
                    delete_flag = true;
                    break;
                }
            }
            if(delete_flag) itr = intervals2.erase(itr);
            else itr++;
        }
    }
public:
    void insert(vector<ll> &cars, vector<vector<ll>> &begin_ids_list, int length) override{
        list<vector<pair<ll, ll>>> intervals;
        for(vector<ll> &begin_ids : begin_ids_list){
            intervals.emplace_back();
            vector<pair<ll, ll>> &ranges = intervals.back();
            for(ll begin_id : begin_ids) ranges.emplace_back(begin_id, begin_id + length - 1);
        }

        auto itr = result_map.find(cars);
        if(itr == result_map.end()) result_map[cars] = intervals;
        else{
            list<vector<pair<ll, ll>>> &main_intervals = itr->second;
            // 尝试逐个插入intervals中的每个值
            for(vector<pair<ll, ll>> &target_ranges : intervals){
                bool insert_flag = true;
                for(auto main_itr = main_intervals.begin(); main_itr != main_intervals.end();){
                    vector<pair<ll, ll>>& current_ranges = *main_itr;
                    bool is_subset = true;// 待插入ranges是否是当前已存储ranges的子集
                    bool is_superset = true;// 待插入ranges是否是当前已存储ranges的超集
                    for(int range_id = 0; range_id < target_ranges.size(); range_id++){
                        pair<ll, ll>& target_range = target_ranges[range_id];
                        pair<ll, ll>& current_range = current_ranges[range_id];
                        if(target_range.first >= current_range.first && target_range.second <= current_range.second)
                            is_superset = false;
                        else if(target_range.first <= current_range.first && target_range.second >= current_range.second)
                            is_subset = false;
                        else{
                            is_subset = false;
                            is_superset = false;
                            break;
                        }
                    }
                    if(is_subset && !is_superset){
                        insert_flag = false;
                        break;
                    }
                    if(is_superset && !is_subset) main_itr = main_intervals.erase(main_itr);
                    else main_itr++;
                }
                if(insert_flag) main_intervals.push_back(target_ranges);
            }
        }
    }

    void de_duplication() override{
        // 为每个result_map中的迭代器分配一个idx
        vector<map<vector<ll>, list<vector<pair<ll, ll>>>>::iterator> idx2itr;
        int curr_idx = 0;
        map<int, vector<int>> length2idx;
        for(auto itr = result_map.begin();itr != result_map.end();itr++){
            int length = itr->first.size();
            auto target_itr = length2idx.find(length);
            if(target_itr == length2idx.end()) length2idx[length] = vector<int>{curr_idx};
            else target_itr->second.push_back(curr_idx);

            idx2itr.push_back(itr);
            curr_idx++;
        }
        // 初始化基础数据结构
        vector<int> idcs;// 按所指向cars集合的大小升序逐层排列
        vector<vector<int>> out_links;// 记录所有点的出边（若a包含b，则b指向a）
        vector<int> starts;// 记录idcs中每层的开始位置
        int item_idx = 0;
        for(auto &entry : length2idx){
            starts.push_back(item_idx);
            item_idx += entry.second.size();
            for(int idx : entry.second){
                idcs.push_back(idx);
                out_links.emplace_back();
            }
        }
        // 初始化倒排表
        unordered_map<ll, unordered_set<int>> inversion_list;
        for(int i = 0; i < idcs.size(); i++){
            const vector<ll> &cars = idx2itr[idcs[i]]->first;
            for(ll car : cars){
                auto itr = inversion_list.find(car);
                if(itr == inversion_list.end()) inversion_list[car] = unordered_set<int>{i};
                else itr->second.insert(i);
            }
        }

        for(int level = 0;level < starts.size();level++){
            int start_idx = starts[level];
            int end_idx = (level == starts.size() - 1) ? idcs.size() - 1 : starts[level + 1] - 1;
            for(int i = start_idx; i <= end_idx; i++){
                const vector<ll>& cars = idx2itr[idcs[i]]->first;
                // 选择涉及key最少的car取交集
                int min_contains = INT_MAX;ll min_car = -1;
                for(ll car : cars){
                    int curr_contains = inversion_list[car].size();
                    if(curr_contains < min_contains){
                        min_contains = curr_contains;
                        min_car = car;
                    }
                }
                if(min_car == -1) cerr << "very weird, from ResultBaseImpl::de_duplication" << endl;

                unordered_set<int> intersection = inversion_list[min_car];
                for(ll car : cars){
                    if(car == min_car) continue;
                    unordered_set<int> &large_set = inversion_list[car];
                    for(auto itr = intersection.begin();itr != intersection.end();){
                        if(large_set.find(*itr) == large_set.end()) itr = intersection.erase(itr);
                        else itr++;
                    }
                }
                intersection.erase(i);
                for(int super_idx : intersection){
                    out_links[i].push_back(super_idx);
                }
                // 删除当前idx在倒排表中的信息（由于idx为拓扑序，故可以安全删除）
                for(ll car : cars) inversion_list[car].erase(i);
            }
        }
        // 倒序逐对去重
        for(int idx = out_links.size() - 1;idx >= 0;idx--){
            vector<int> &supers = out_links[idx];
            for(int super_idx : supers) de_duplication(idx2itr, idcs[super_idx], idcs[idx]);
        }
    }

    // 一系列结果打印函数
    void print_contents() override{
        int n_objects = 0;
        int n_convoys = 0;
        for(auto &entry : result_map){
            if(entry.second.empty()) continue;
            n_objects++;
            n_convoys += entry.second.size();
        }
        cout << "Total number of discovered convoys: " << n_convoys << ", object combinations: " << n_objects<<" - ";
    }
    void print_contents(MiningTree &theTree){
        int n_objects = 0;
        int n_convoys = 0;
        for(auto &entry : result_map){
            if(entry.second.empty()) continue;
            n_objects++;
            n_convoys += entry.second.size();
            string buffer;
            // 填充key到buffer中
            buffer.push_back('{');
            for(ll object_id : entry.first) buffer += to_string(object_id + 1) + ",";// 真实车辆的编号从1开始
            buffer.pop_back();
            buffer.push_back('}');
            buffer += " : ";
            // 填充value的内容到buffer中
            buffer.push_back('[');
            for(vector<pair<ll, ll>>& ranges : entry.second){
                pair<ll, ll>& first_range = ranges.front();
                string begin_text_id = to_string(first_range.first);
                buffer += "(" + begin_text_id + ")";
                for(ll loc_id = first_range.first;loc_id <= first_range.second;loc_id++){
                    buffer += to_string(theTree.text[loc_id].camera) + "-";
                }
                buffer.pop_back();
                buffer.push_back(',');
            }
            buffer.pop_back();
            buffer.push_back(']');
            cout<<buffer<<endl;
        }
        cout << "Total number of discovered convoys: " << n_convoys << ", object combinations: " << n_objects << endl;
    }
    void dump_contents(MiningTree &theTree, string dump_path){
        ofstream ofs;
        ofs.open(dump_path, ios::out);
        ofs<<"Objects,Path"<<endl;
        for(auto &entry : result_map){
            const vector<ll> &key = entry.first;
            list<vector<pair<ll, ll>>> &intervals = entry.second;
            if(intervals.empty()) continue;
            string object_str;
            // 填充key到buffer中
            for(ll object_id : key) object_str += to_string(object_id + 1) + " ";// 真实车辆的编号从1开始
            object_str.pop_back();
            // 填充value的内容到buffer中
            for(vector<pair<ll, ll>> &ranges : intervals){
                string buffer;
                pair<ll, ll> &first_range = ranges.front();
                for(ll loc_id = first_range.first;loc_id <= first_range.second;loc_id++){
                    buffer += to_string(theTree.text[loc_id].camera) + " ";
                }
                buffer.pop_back();
                ofs<<object_str<<","<<buffer<<endl;
            }
        }
        ofs.close();
    }

};

class ResultTwoMapImpl : public Result{
private:
    MiningTree &mining_tree;
    map<vector<ll>, map<vector<ll>, list<vector<ll>>>> path2cars;
    map<vector<ll>, set<vector<ll>>> cars2path;

    void add_to_car_map(vector<ll> &cars, list<vector<ll>> &begin_ids_list, int path_length){
        auto itr = cars2path.find(cars);
        if(itr == cars2path.end()){
            set<vector<ll>> paths;
            for(vector<ll> &begin_ids : begin_ids_list){
                begin_ids.push_back(path_length);
                paths.insert(begin_ids);
            }
            cars2path[cars] = paths;
        }
        else{
            int car_length = cars.size();
            for(vector<ll> &begin_ids : begin_ids_list){
                vector<pair<ll, ll>> target_ranges;
                for(int i = 0;i < car_length;i++) target_ranges.emplace_back(begin_ids[i], begin_ids[i] + path_length - 1);

                bool insert_flag = true;
                set<vector<ll>> &paths = itr->second;
                for(auto path_itr = paths.begin();path_itr != paths.end();){
                    const vector<ll> &curr_path = *path_itr;
                    ll curr_length = curr_path.back();

                    bool is_subset = true; //待插入path是否是当前已存储path的子集
                    bool is_superset = true; //待插入path是否是当前已存储path的超集
                    for(int i = 0;i < car_length;i++){
                        pair<ll, ll> &target_range = target_ranges[i];
                        ll curr_begin = curr_path[i], curr_end = curr_begin + curr_length - 1;
                        if(target_range.first >= curr_begin && target_range.second <= curr_end) is_superset = false;
                        else if(target_range.first <= curr_begin && target_range.second >= curr_end) is_subset = false;
                        else{
                            is_subset = false;
                            is_superset = false;
                            break;
                        }
                    }
                    if(is_subset && !is_superset){
                        insert_flag = false;
                        break;
                    }
                    if(is_superset && !is_subset) path_itr = paths.erase(path_itr);
                    else path_itr++;
                }
                if(insert_flag){
                    begin_ids.push_back(path_length);
                    paths.insert(begin_ids);
                }
            }
        }
    }
    void remove_from_car_map(const vector<ll> &cars, vector<ll> &begin_ids, int path_length){
        auto itr = cars2path.find(cars);
        if(itr == cars2path.end()) return;
        else{
            begin_ids.push_back(path_length);
            itr->second.erase(begin_ids);
            if(itr->second.empty()) cars2path.erase(itr);
        }
    }
public:
    explicit ResultTwoMapImpl(MiningTree &theTree) : mining_tree(theTree){
        redundant_flag = true;
    }

    void insert(vector<ll> &cars, vector<vector<ll>> &begin_ids_vt, int length) override{
        list<vector<ll>> begin_ids_list(begin_ids_vt.begin(), begin_ids_vt.end());
        vector<ll> path;
        ll loc_begin = begin_ids_vt.front().front(), loc_end = loc_begin + length - 1;
        for(ll loc_id = loc_begin; loc_id <= loc_end; loc_id++) path.push_back(mining_tree.text[loc_id].camera);

        auto path_itr = path2cars.find(path);
        if(path_itr == path2cars.end()){
            path2cars[path] = map<vector<ll>, list<vector<ll>>>{{cars, begin_ids_list}};
            add_to_car_map(cars, begin_ids_list, length);
        }
        else{
            map<vector<ll>, list<vector<ll>>> &cars_info_map = path_itr->second;
            auto insert_itr = cars_info_map.end();
            for(auto cm_itr = cars_info_map.begin(); cm_itr != cars_info_map.end();){
                const vector<ll> &crt_cars = cm_itr->first;

                const vector<ll> *large_cars = &cars, *small_cars = &crt_cars;
                if(large_cars->size() <= small_cars->size()) swap(large_cars, small_cars);

                vector<int> pos_list;//small_cars各元素在large_cars中的位置
                for(int small_idx = 0, large_idx = 0;large_idx < large_cars->size();large_idx++){
                    ll small_car = small_cars->at(small_idx), large_car = large_cars->at(large_idx);
                    if(small_car < large_car) break;
                    else if(small_car == large_car){
                        pos_list.push_back(large_idx);
                        small_idx++;
                        if(small_idx == small_cars->size()) break;
                    }
                }
                if(pos_list.size() != small_cars->size()) cm_itr++;
                else{
                    if(large_cars == &crt_cars){
                        set<vector<ll>> begins_set;
                        if(large_cars->size() == small_cars->size()){
                            insert_itr = cm_itr;
                            for(vector<ll> &crt_begins : cm_itr->second) begins_set.insert(crt_begins);
                        }
                        else{
                            for(vector<ll> &crt_begins : cm_itr->second){
                                vector<ll> temp_begins;
                                for(int pos : pos_list) temp_begins.push_back(crt_begins[pos]);
                                begins_set.insert(temp_begins);
                            }
                        }
                        for(auto itr = begin_ids_list.begin();itr != begin_ids_list.end();){
                            if(begins_set.find(*itr) != begins_set.end()) itr = begin_ids_list.erase(itr);
                            else itr++;
                        }
                        if(begin_ids_list.empty()) break;
                        cm_itr++;
                    }
                    else{
                        set<vector<ll>> begins_set;
                        for(vector<ll> &begins : begin_ids_list){
                            vector<ll> temp_begins;
                            for(int pos : pos_list) temp_begins.push_back(begins[pos]);
                            begins_set.insert(temp_begins);
                        }
                        list<vector<ll>> &crt_begins_list = cm_itr->second;
                        for(auto itr = crt_begins_list.begin();itr != crt_begins_list.end();){
                            if(begins_set.find(*itr) != begins_set.end()){
                                vector<ll> &begins = *itr;
                                remove_from_car_map(crt_cars, begins, length);
                                itr = crt_begins_list.erase(itr);
                            }
                            else itr++;
                        }
                        if(crt_begins_list.empty()) cm_itr = cars_info_map.erase(cm_itr);
                        else cm_itr++;
                    }
                }
            }
            if(begin_ids_list.empty()) return;
            if(insert_itr == cars_info_map.end()) insert_itr = cars_info_map.find(cars);

            if(insert_itr == cars_info_map.end()) cars_info_map[cars] = begin_ids_list;
            else{
                list<vector<ll>> &insert_list = insert_itr->second;
                insert_list.insert(insert_list.end(), begin_ids_list.begin(), begin_ids_list.end());
            }
            add_to_car_map(cars, begin_ids_list, length);
        }
    }

    void de_duplication() override {
        // The main processing of hashing-based duplicate elimination can be performed during result insertion
    }

    void print_contents() override {
        int n_objects = 0;
        int n_convoys = 0;
        for(auto &entry : cars2path){
            if(entry.second.empty()) continue;
            n_objects++;
            n_convoys += entry.second.size();
        }
        cout << "Total number of discovered convoys: " << n_convoys << ", object combinations: " << n_objects<<" - ";
    }
    void print_contents(MiningTree &theTree) {
        int n_objects = 0;
        int n_convoys = 0;
        for(auto &entry : cars2path){
            if(entry.second.empty()) continue;
            n_objects++;
            n_convoys += entry.second.size();
            string buffer;
            // 填充key到buffer中
            buffer.push_back('{');
            for(ll object_id : entry.first) buffer += to_string(object_id + 1) + ",";// 真实车辆的编号从1开始
            buffer.pop_back();
            buffer.push_back('}');
            buffer += " : ";
            // 填充value的内容到buffer中
            buffer.push_back('[');
            for(const vector<ll> &path : entry.second){
                ll start = path.front(), end = start + path.back() - 1;
                for(ll loc_id = start;loc_id <= end;loc_id++){
                    buffer += to_string(theTree.text[loc_id].camera) + "-";
                }
                buffer.pop_back();
                buffer.push_back(',');
            }
            buffer.pop_back();
            buffer.push_back(']');
            cout<<buffer<<endl;
        }
        cout << "Total number of discovered convoys: " << n_convoys << ", object combinations: " << n_objects << endl;
    }
    void dump_contents(MiningTree &theTree, string dump_path){
        ofstream ofs;
        ofs.open(dump_path, ios::out);
        ofs<<"Objects,Path"<<endl;
        for(auto &entry : cars2path){
            const vector<ll> &key = entry.first;
            set<vector<ll>> &paths = entry.second;
            if(paths.empty()) continue;
            string object_str;
            // 填充key到buffer中
            for(ll object_id : key) object_str += to_string(object_id + 1) + " ";// 真实车辆的编号从1开始
            object_str.pop_back();
            // 填充value的内容到buffer中
            for(const vector<ll> &path : paths){
                string buffer;
                ll start = path.front(), end = start + path.back() - 1;
                for(ll loc_id = start;loc_id <= end;loc_id++){
                    buffer += to_string(theTree.text[loc_id].camera) + " ";
                }
                buffer.pop_back();
                ofs<<object_str<<","<<buffer<<endl;
            }
        }
        ofs.close();
    }
};

#endif //PROJECT_RESULT_H
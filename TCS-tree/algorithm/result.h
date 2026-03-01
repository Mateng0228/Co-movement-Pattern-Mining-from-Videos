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
    bool redundant_flag = true;

    virtual ~Result() = default;
    virtual void insert(vector<ll> &cars, vector<vector<ll>> &begin_ids_list, int length) = 0;
    virtual void de_duplication() = 0;
    virtual void print_contents() = 0;
    virtual void dump_contents(MiningTree &theTree, string dump_path) = 0;
};

class ResultBaseImpl : public Result{
private:
    map<vector<ll>, list<vector<pair<ll, ll>>>> result_map;
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
            for(vector<pair<ll, ll>> &target_ranges : intervals)
                main_intervals.push_back(target_ranges);
        }
    }

    void de_duplication() override{
        map<ll, vector<ll>> car_map;
        vector<vector<ll>> car_vec;
        for(auto it = result_map.begin(); it != result_map.end(); it++) {
            car_vec.push_back(it->first);
            for(ll car : it->first) {
                if(car_map.find(car) == car_map.end()) {
                    vector<ll> id_set;
                    id_set.push_back(car_vec.size() - 1);
                    car_map[car] = id_set;
                }
                else {
                    car_map[car].push_back(car_vec.size() - 1);
                }
            }
        }

        for(ll i = 0; i < car_vec.size(); i++) {
            vector<ll> cars = car_vec[i];
            vector<ll> result = car_map[cars[0]];
            for(ll car : cars) {
                vector<ll> intersec_set = car_map[car];
                vector<ll> tmp;
                for(ll j = 0; j < result.size(); j++) {
                    for(ll k = 0; k < intersec_set.size(); k++) {
                        if(result[j] == intersec_set[k]) {
                            tmp.push_back(result[j]);
                        }
                    }
                }
                result = tmp;
            }
            for(ll id : result) {
                list<vector<pair<ll, ll>>> &parent_path_list = result_map[car_vec[id]];
                vector<ll> parent_cars = car_vec[id];
                list<vector<pair<ll, ll>>> &child_path_list = result_map[car_vec[i]];
                vector<vector<pair<ll, ll>>> child_path_vec;
                vector<bool> is_dominate(child_path_list.size(),0);
                for(auto child_path : child_path_list) {
                    child_path_vec.push_back(child_path);
                }

                int ans = 0;
                for(ll m = 0; m < child_path_vec.size();) {
                    vector<pair<ll, ll>> child_path = child_path_vec[m];
                    bool is_ok = 0;
                    for(vector<pair<ll,ll>> parent_path : parent_path_list) {
                        ll dominate_len = 0;
                        for(ll j = 0; j < child_path.size(); j++) {
                            for(ll k = 0; k < parent_path.size(); k++) {
                                if(cars[j] == parent_cars[k]) {
                                    if(child_path[j].first >= parent_path[k].first && child_path[j].second <= parent_path[k].second) {
                                        dominate_len ++;
                                    }
                                }
                            }
                        }
                        if(dominate_len == cars.size()) {
                            is_ok = 1;
                        }
                    }
                    if(is_ok == 1) {
                        child_path_vec.erase(child_path_vec.begin() + m);
                    } else {
                        m++;
                    }
                }
                list<vector<pair<ll, ll>>> new_child_path_list;
                for(ll j = 0; j < child_path_vec.size(); j++) {
                    new_child_path_list.push_back(child_path_vec[j]);
                }
                if(id != i){
                    result_map[car_vec[i]] = new_child_path_list;
                }
            }
        }

        for(auto it = result_map.begin(); it != result_map.end(); it++) {
            vector<ll> cars = it->first;
            //list<vector<pair<ll,ll>>> path_list = it->second;
            vector<vector<pair<ll,ll>>> path_list;
            for(auto path : it->second) {
                path_list.push_back(path);
            }
            vector<bool> is_dominate(path_list.size(),0);
            for(ll i = 0; i < path_list.size(); i++) {

                for(ll j = i + 1; j < path_list.size(); j++) {
                    ll dominate_i_len = 0;
                    ll dominate_j_len = 0;
                    for(ll k = 0; k < path_list[i].size(); k++) {
                        if(path_list[i][k].first >= path_list[j][k].first && path_list[i][k].second <= path_list[j][k].second) {
                            dominate_i_len ++;
                        }
                        else if(path_list[i][k].first <= path_list[j][k].first && path_list[i][k].second >= path_list[j][k].second){
                            dominate_j_len ++;
                        }
                        else{
                        }
                    }
                    if(dominate_i_len == cars.size()) {
                        is_dominate[i] = 1;
                    }
                    if(dominate_j_len == cars.size()) {
                        is_dominate[j] = 1;
                    }

                }
            }
            list<vector<pair<ll,ll>>> new_path_list;
            for(ll i = 0; i < path_list.size(); i++) {
                if(!is_dominate[i]) {
                    new_path_list.push_back(path_list[i]);
                }
            }
            it->second = new_path_list;
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
    void dump_contents(MiningTree &theTree, string dump_path) override{
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

    void remove_from_car_map(const vector<ll> &cars, const vector<ll> &begin_ids, int path_length){
        auto itr = cars2path.find(cars);
        if(itr == cars2path.end()) return;
        else{
            vector<ll> tmp_begin_ids(begin_ids);
            tmp_begin_ids.push_back(path_length);
            itr->second.erase(tmp_begin_ids);
            if(itr->second.empty()) cars2path.erase(itr);
        }
    }
public:
    explicit ResultTwoMapImpl(MiningTree &theTree) : mining_tree(theTree){
        redundant_flag = true;
    }

    void insert(vector<ll> &cars, vector<vector<ll>> &begin_ids_vt, int length) override{
        vector<ll> path;
        ll loc_begin = begin_ids_vt.front().front(), loc_end = loc_begin + length - 1;
        for(ll loc_id = loc_begin; loc_id <= loc_end; loc_id++) path.push_back(mining_tree.text[loc_id].camera);
        list<vector<ll>> begin_ids_lst(begin_ids_vt.begin(), begin_ids_vt.end());

        // add into first_map
        auto path_itr = path2cars.find(path);
        if(path_itr == path2cars.end()) path2cars[path] = map<vector<ll>, list<vector<ll>>>{{cars, begin_ids_lst}};
        else{
            map<vector<ll>, list<vector<ll>>> &cars_info_map = path_itr->second;
            auto insert_itr = cars_info_map.find(cars);
            if(insert_itr == cars_info_map.end()) cars_info_map[cars] = begin_ids_lst;
            else{
                list<vector<ll>> &total_lst = insert_itr->second;
                total_lst.insert(total_lst.end(), begin_ids_lst.begin(), begin_ids_lst.end());
            }
        }
        // add into second_map
        auto car_itr = cars2path.find(cars);
        if(car_itr == cars2path.end()){
            set<vector<ll>> paths;
            for(vector<ll> &begin_ids : begin_ids_lst){
                begin_ids.push_back(length);
                paths.insert(begin_ids);
            }
            cars2path[cars] = paths;
        }
        else{
            set<vector<ll>> &path_st = car_itr->second;
            for(vector<ll> &begin_ids : begin_ids_lst){
                begin_ids.push_back(length);
                path_st.insert(begin_ids);
            }
        }
    }

    void de_duplication() override {
        // update first_map
        for(auto &path_map_entry : path2cars){
            int path_length = path_map_entry.first.size();
            map<vector<ll>, list<vector<ll>>> &cars_map = path_map_entry.second;
            vector<map<vector<ll>, list<vector<ll>>>::iterator> itr_lst;
            for(auto it = cars_map.begin(); it != cars_map.end(); it++) itr_lst.push_back(it);
            sort(itr_lst.begin(), itr_lst.end(), [](const auto &it1, const auto &it2){
                return it1->first.size() > it2->first.size();
            });
            for(int i = 0; i < itr_lst.size() - 1; i++){
                const vector<ll> &cars_i = itr_lst[i]->first;
                list<vector<ll>> &target_lst = itr_lst[i]->second;
                if(target_lst.empty()) continue;
                for(int j = i + 1; j < itr_lst.size(); j++){
                    auto itr_j = itr_lst[j];
                    const vector<ll> &cars_j = itr_j->first;
                    list<vector<ll>> &check_lst = itr_j->second;
                    if(cars_i.size() == cars_j.size() or check_lst.empty()) continue;
                    if(includes(cars_i.begin(), cars_i.end(), cars_j.begin(), cars_j.end())){
                        vector<int> car_ids;// cars_j包含的car分别在cars_i中的位置
                        int pi = 0, pj = 0;
                        while(pj != cars_j.size()){
                            if(pi == cars_i.size()){cerr << "cars_i 并不包含 cars_j" << endl; return; }
                            if(cars_i[pi] != cars_j[pj]) pi++;
                            else{
                                car_ids.push_back(pi);
                                pi++; pj++;
                            }
                        }
                        for(auto check_itr = check_lst.begin(); check_itr != check_lst.end();){
                            vector<ll> &check_begins = *check_itr;
                            bool hit_flag = false;
                            for(vector<ll> &target_begins : target_lst){
                                vector<ll> search_begins;
                                for(int car_id : car_ids) search_begins.push_back(target_begins[car_id]);
                                if(search_begins == check_begins){
                                    hit_flag = true;
                                    break;
                                }
                            }
                            if(hit_flag){
                                remove_from_car_map(cars_j, check_begins, path_length);
                                check_itr = check_lst.erase(check_itr);
                            }
                            else check_itr++;
                        }
                    }
                }
            }

        }
        // update second_map
        for(auto cars_it = cars2path.begin(); cars_it != cars2path.end();){
            int car_length = cars_it->first.size();
            set<vector<ll>> &path_st = cars_it->second;
            vector<set<vector<ll>>::iterator> itr_lst;
            for(auto it = path_st.begin(); it != path_st.end(); it++) itr_lst.push_back(it);
            sort(itr_lst.begin(), itr_lst.end(), [](const auto &it1, const auto &it2){
                return it1->back() < it2->back();
            });
            vector<set<vector<ll>>::iterator> erase_lst;
            for(int i = 0; i < itr_lst.size() - 1; i++){
                const vector<ll> &target_path = *(itr_lst[i]);ll target_length = target_path.back();
                for(int j = i + 1; j < itr_lst.size(); j++){
                    const vector<ll> &check_path = *(itr_lst[j]);ll check_length = check_path.back();
                    bool included_flag = true;
                    for(int car_id = 0; car_id < car_length; car_id++){
                        ll target_begin = target_path[car_id], target_end = target_begin + target_length - 1;
                        ll check_begin = check_path[car_id], check_end = check_begin + check_length - 1;
                        if(!(target_begin >= check_begin && target_end <= check_end)){
                            included_flag = false;
                            break;
                        }
                    }
                    if(included_flag){
                        erase_lst.push_back(itr_lst[i]);
                        break;
                    }
                }
            }
            for(auto &erase_itr : erase_lst){
                path_st.erase(erase_itr);
            }
            if(path_st.empty()) cars2path.erase(cars_it);
            else cars_it++;
        }
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
    void dump_contents(MiningTree &theTree, string dump_path) override{
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
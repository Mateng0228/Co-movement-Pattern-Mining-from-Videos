#ifndef PROJECT_UTILS_H
#define PROJECT_UTILS_H
#include <sys/types.h>
#include <dirent.h>
#include <fstream>
#include <cstring>
#include <vector>
#include <map>
#include <set>
#include <iostream>
#include <algorithm>
#include "algorithm2/position.h"

using namespace std;
// 获得某文件夹下所有的子文件（不递归获得子文件夹内的文件）
void get_subfiles(string path, vector<string>& filePaths) {
    vector<string> file_names;
    DIR *pDir;
    struct dirent* ptr;
    if(!(pDir = opendir(path.c_str()))) return;
    while((ptr = readdir(pDir)) != nullptr) {
        if(strcmp(ptr->d_name, ".") != 0 && strcmp(ptr->d_name, "..") != 0)
            file_names.emplace_back(ptr->d_name);
    }
    closedir(pDir);

    map<int, int> value2idx;
    for(int idx = 0;idx < file_names.size();idx++){
        string &file_name = file_names[idx];
        int value = stoi(file_name.substr(0, file_name.find('.')));
        value2idx[value] = idx;
    }
    path += '/';
    for(auto &pair : value2idx){
        string file_path = path + file_names[pair.second];
        filePaths.push_back(file_path);
    }
}

// 将文件中的数据填充到指定数据结构中
void fill_data(vector<long long> *strings, vector<pair<double, double>> *intervals, vector<string> paths){
    int n_strings = paths.size();
    for(int idx = 0;idx < n_strings;idx++){
        vector<long long> objectIds;
        vector<pair<double, double>> objectIntervals;
        ifstream fin;
        fin.open(paths[idx], ios::in);
        string buff;
        while(getline(fin, buff)){
            int cut1 = -1, cut2 = -1;
            for(int i = 0;i<buff.size();i++){
                if(buff[i] == ','){
                    if(cut1 == -1)cut1 = i;
                    else if(cut2 == -1)cut2 = i;
                }
            }
            string part1 = buff.substr(0, cut1);
            string part2 = buff.substr(cut1+1, cut2-cut1-1);
            string part3 = buff.substr(cut2+1, buff.size()-cut2-1);
            long long objectId = stoll(part1);
            double left = stod(part2), right = stod(part3);
            objectIds.push_back(objectId);
            objectIntervals.emplace_back(left, right);
            strings[idx] = objectIds;
            intervals[idx] = objectIntervals;
        }
        fin.close();
    }
}

// 将文件中的数据填充到指定数据结构中
void fill_data(vector<position> *arr_positions, vector<string> paths){
    int arr_size = paths.size();
    for(int idx = 0; idx < arr_size; idx++){
        arr_positions[idx] = vector<position>();

        ifstream fin;
        fin.open(paths[idx], ios::in);
        string buff;
        while(getline(fin, buff)){
            int cut1 = -1, cut2 = -1;
            for(int i = 0;i < buff.size();i++){
                if(buff[i] == ','){
                    if(cut1 == -1)cut1 = i;
                    else if(cut2 == -1)cut2 = i;
                }
            }
            string part1 = buff.substr(0, cut1);
            string part2 = buff.substr(cut1+1, cut2-cut1-1);
            string part3 = buff.substr(cut2+1, buff.size()-cut2-1);
            long long camera = stoll(part1);
            double interval_left = stod(part2), interval_right = stod(part3);

            arr_positions[idx].emplace_back(camera, interval_left, interval_right);
        }
        fin.close();
    }
}

int count_unique(vector<int>& list) {
    if(list.empty()) return 0;

    vector<int> temp_list = list;
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

void deduplicate(set<vector<int>> &candidates){
    if(candidates.empty()) return;

    vector<set<vector<int>>::iterator> itr_lst;
    vector<vector<int>> stub_candidates;
    for(auto it = candidates.begin(); it != candidates.end(); it++)
        itr_lst.push_back(it);
    sort(itr_lst.begin(), itr_lst.end(), [](const auto &itr1, const auto &itr2){
        return itr1->size() < itr2->size();
    });
    for(auto &itr : itr_lst){
        stub_candidates.emplace_back(itr->begin(), itr->end());
        vector<int> &back_candidate = stub_candidates.back();
        sort(back_candidate.begin(), back_candidate.end());
    }
    vector<set<vector<int>>::iterator> erase_lst;
    for(int i = 0; i < itr_lst.size() - 1; i++){
        const vector<int> &check_cand = stub_candidates[i];
        auto check_size = check_cand.size();
        for(int j = i + 1; j < itr_lst.size(); j++){
            const vector<int> &temp_cand = stub_candidates[j];
            if(check_size == temp_cand.size()) continue;
            if(includes(temp_cand.begin(), temp_cand.end(), check_cand.begin(), check_cand.end())){
                erase_lst.push_back(itr_lst[i]);
                break;
            }
        }
    }
    for(auto &erase_itr : erase_lst){
        candidates.erase(erase_itr);
    }
}

void deduplicate(set<pair<vector<int>, int>> &candidates){
    if(candidates.empty()) return;

    vector<set<pair<vector<int>, int>>::iterator> itr_lst;
    vector<pair<vector<int>, int>> stub_candidates;
    for(auto it = candidates.begin(); it != candidates.end(); it++) itr_lst.push_back(it);
    sort(itr_lst.begin(), itr_lst.end(), [](const auto &itr1, const auto &itr2){
        return itr1->first.size() < itr2->first.size();
    });
    for(auto &itr : itr_lst){
        stub_candidates.emplace_back(vector<int>(itr->first.begin(), itr->first.end()), itr->second);
        auto &back_candidate = stub_candidates.back();
        sort(back_candidate.first.begin(), back_candidate.first.end());
    }
    vector<set<pair<vector<int>, int>>::iterator> erase_lst;
    for(int i = 0; i < itr_lst.size() - 1; i++){
        const auto &check_cand = stub_candidates[i];
        auto check_size = check_cand.first.size();
        for(int j = i + 1; j < itr_lst.size(); j++){
            const auto &temp_cand = stub_candidates[j];
            if(check_size == temp_cand.first.size()) continue;
            bool is_include = includes(temp_cand.first.begin(), temp_cand.first.end(), check_cand.first.begin(), check_cand.first.end());
            if(is_include && check_cand.second >= temp_cand.second){
                erase_lst.push_back(itr_lst[i]);
                break;
            }
        }
    }
    for(auto &erase_itr : erase_lst){
        candidates.erase(erase_itr);
    }
}

#endif //PROJECT_UTILS_H

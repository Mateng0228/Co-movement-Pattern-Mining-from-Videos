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
#include "algorithm/position.h"

using namespace std;
// Obtain all sub-files under given folder
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

// Fill the data in files into the specified data structure
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

// Fill the data in files into the specified data structure
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

#endif //PROJECT_UTILS_H

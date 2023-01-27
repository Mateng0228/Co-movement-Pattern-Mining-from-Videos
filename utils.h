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

// 检测两文件中的convoy是否一致(文件中convoy的格式为"1 2 3,A B C")
void check(string path1, string path2){
    map<set<int>, set<vector<int>>> convoy1;
    ifstream fin1;
    fin1.open(path1, ios::in);
    string buffer;
    getline(fin1, buffer);
    while(getline(fin1, buffer)){
        set<int> objects;
        vector<int> sequence;
        int pos = buffer.find(',');
        string object_str = buffer.substr(0, pos);
        string sequence_str = buffer.substr(pos + 1, buffer.length() - 1 - pos);
        object_str += " ";
        int left = 0, right = object_str.find(' ', 0);
        while(right != std::string::npos){
            objects.insert(stoi(object_str.substr(left, right - left)));
            left = right + 1;
            right = object_str.find(' ', left);
        }
        sequence_str += " ";
        left = 0, right = sequence_str.find(' ', 0);
        while(right != std::string::npos){
            sequence.push_back(stoi(sequence_str.substr(left, right - left)));
            left = right + 1;
            right = sequence_str.find(' ', left);
        }
        if(convoy1.find(objects) == convoy1.end()) convoy1[objects] = set<vector<int>>();
        convoy1[objects].insert(sequence);
    }
    fin1.close();

    map<set<int>, set<vector<int>>> convoy2;
    ifstream fin2;
    fin2.open(path2, ios::in);
    getline(fin2, buffer);
    while(getline(fin2, buffer)){
        set<int> objects;
        vector<int> sequence;
        int pos = buffer.find(',');
        string object_str = buffer.substr(0, pos);
        string sequence_str = buffer.substr(pos + 1, buffer.length() - 1 - pos);
        object_str += " ";
        int left = 0, right = object_str.find(' ', 0);
        while(right != std::string::npos){
            objects.insert(stoi(object_str.substr(left, right - left)));
            left = right + 1;
            right = object_str.find(' ', left);
        }
        sequence_str += " ";
        left = 0, right = sequence_str.find(' ', 0);
        while(right != std::string::npos){
            sequence.push_back(stoi(sequence_str.substr(left, right - left)));
            left = right + 1;
            right = sequence_str.find(' ', left);
        }
        if(convoy2.find(objects) == convoy2.end()) convoy2[objects] = set<vector<int>>();
        convoy2[objects].insert(sequence);
    }
    fin2.close();

    map<set<int>, set<vector<int>>> diff_map1, diff_map2;
    for(auto &p : convoy1){
        const set<int>& objects1 = p.first;
        if(convoy2.find(objects1) == convoy2.end()) diff_map1[objects1] = p.second;
        else{
            set<vector<int>>& sequence_list1 = p.second;
            set<vector<int>>& sequence_list2 = convoy2[objects1];
            for(const vector<int>& sequence1 : sequence_list1){
                if(sequence_list2.find(sequence1) == sequence_list2.end()){
                    if(diff_map1.find(objects1) == diff_map1.end()) diff_map1[objects1] = set<vector<int>>();
                    diff_map1[objects1].insert(sequence1);
                }
                else sequence_list2.erase(sequence1);
            }
            if(!sequence_list2.empty()) diff_map2[objects1] = sequence_list2;
        }
        convoy2.erase(objects1);
    }
    diff_map2.insert(convoy2.begin(), convoy2.end());

    cout<<"convoy1 differences:--------------------"<<endl;
    int counts = 0;
    for(auto &p : diff_map1){
        cout<<"{";
        for(int object : p.first) cout<<object<<",";
        cout<<"} : [";
        for(const vector<int>& sequence : p.second){
            counts++;
            for(int item : sequence) cout<<item<<"-";
            cout<<", ";
        }
        cout<<"]"<<endl;
    }
    cout<<counts<<endl;
    counts = 0;
    cout<<"convoy2 differences:--------------------"<<endl;
    for(auto &p : diff_map2){
        cout<<"{";
        for(int object : p.first) cout<<object<<",";
        cout<<"} : [";
        for(const vector<int>& sequence : p.second){
            counts++;
            for(int item : sequence) cout<<item<<"-";
            cout<<", ";
        }
        cout<<"]"<<endl;
    }
    cout<<counts<<endl;
}

#endif //PROJECT_UTILS_H

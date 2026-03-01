#ifndef EXPERIMENT_INPUT_HANDLER_H
#define EXPERIMENT_INPUT_HANDLER_H

#include <dirent.h>
#include <fstream>
#include <cstring>
#include <map>
#include <iostream>
#include "structs.h"

using namespace std;

class InputHandler{
public:
    using update_item = pair<int, Position>;
    int N = 0;

    explicit InputHandler(string dir_path){read_file(dir_path);}

    vector<pair<int, Position>>& get_update_data(int n = -1);
    map<int, Path>& get_history_map() {return history_map;};
    map<int, Path>& get_all_map() {return stub_map;};
private:
    map<int, Path> stub_map;
    map<int, Path> history_map;
    vector<update_item> update_items;

    void read_file(const string &dir_path);
};


vector<pair<int, Position>>& InputHandler::get_update_data(int n){
    if(n == -1) n = N;
    n = min(n, N);
    update_items = {};
    history_map = {};

    vector<update_item> items;
    for(const auto &stub_entry : stub_map){
        int oid = stub_entry.first;
        const vector<Position> &positions = stub_entry.second.positions;
        for(auto &position : positions){
            items.emplace_back(oid, position);
        }
    }
    sort(items.begin(), items.end(), [](const update_item &item1, const update_item &item2){
        return item1.second.te < item2.second.te;
    });

    update_items.insert(update_items.end(), items.begin(), items.begin() + n);
    for(const auto &update_item : update_items){
        int oid = update_item.first;
        auto it = history_map.find(oid);
        if(it == history_map.end())
            history_map.insert(make_pair(oid, Path(oid, {update_item.second})));
        else it->second.positions.push_back(update_item.second);
    }
    return update_items;
}


void InputHandler::read_file(const string &dir_path){
    N = 0;
    vector<string> file_names;
    DIR *pDir;
    struct dirent* ptr;
    if(!(pDir = opendir(dir_path.c_str()))) return;
    while((ptr = readdir(pDir)) != nullptr) {
        if(strcmp(ptr->d_name, ".") != 0 && strcmp(ptr->d_name, "..") != 0)
            file_names.emplace_back(ptr->d_name);
    }
    closedir(pDir);

    for(auto &file_name : file_names){
        string object_name = file_name.substr(0, file_name.find('.'));
        int object_id = stoi(object_name); // 默认直接以object_name的数值作为object_id
        vector<Position> traj;
        ifstream fin;
        fin.open(dir_path + "/" + file_name, ios::in);
        string buff;
        while(getline(fin, buff)){
            int cut1 = -1, cut2 = -1;
            for(int i = 0; i < buff.size(); i++){
                if(buff[i] == ','){
                    if(cut1 == -1) cut1 = i;
                    else if(cut2 == -1) cut2 = i;
                }
            }
            string part1 = buff.substr(0, cut1);
            string part2 = buff.substr(cut1 + 1, cut2 - cut1 - 1);
            string part3 = buff.substr(cut2 + 1, buff.size() - cut2 - 1);
            ll camera_id = stoll(part1);
            double begin_time = stod(part2), end_time = stod(part3);
            traj.emplace_back(camera_id, begin_time, end_time);
            N += 1;
        }
        fin.close();
        stub_map.insert(make_pair(object_id, Path(object_id, traj)));
    }
}

#endif //EXPERIMENT_INPUT_HANDLER_H

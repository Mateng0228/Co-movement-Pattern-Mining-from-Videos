#ifndef EXPERIMENT_BASELINE_H
#define EXPERIMENT_BASELINE_H

#include "Offline/query.h"
#include "structs.h"
#include "result.h"

class Baseline{
private:
    int m, k; double eps;
    vector<pair<int, Position>> buffer;
    OfflineResult *res_ptr = nullptr;
public:
    int N, buffer_size;
    map<int, Path> data_map; // oid to path

    ~Baseline() {delete res_ptr;}
    Baseline(int m, int k, double eps) : m(m), k(k), eps(eps), N(0), buffer_size(1) {}
    Baseline(int m, int k, double eps, int buffer_size) : m(m), k(k), eps(eps), N(0), buffer_size(buffer_size) {}
    void update(int oid, Position position, bool immediate = false);
    void print();
};

void Baseline::update(int oid, Position position, bool immediate) {
    buffer.emplace_back(oid, position);
    if(!immediate && buffer.size() != buffer_size) return;

    N += static_cast<int>(buffer.size());
    for(const auto &entry : buffer){
        int tmp_oid = entry.first;
        const Position &tmp_position = entry.second;
        auto tmp_it = data_map.find(tmp_oid);
        if(tmp_it == data_map.end()) data_map.insert(make_pair(tmp_oid, Path(tmp_oid, {tmp_position})));
        else tmp_it->second.positions.push_back(tmp_position);
    }
    buffer = {};

    delete res_ptr;
    TCS_Query q(data_map, m, k, eps);
    res_ptr = q.query();
}

void Baseline::print(){
    if(res_ptr == nullptr) cout<<"Total number of discovered patterns: 0, object combinations: 0"<<endl;
    else res_ptr->print_contents();
}

#endif //EXPERIMENT_BASELINE_H

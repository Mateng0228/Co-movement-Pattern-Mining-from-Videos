#ifndef EXPERIMENT_MINER_H
#define EXPERIMENT_MINER_H

#include <map>
#include <unordered_map>
#include <random>
#include <deque>
#include "structs.h"
#include "result.h"

using namespace std;

class TreeMiner{
public:
    int m, k; double eps;
    Result result;

    TreeMiner(int m, int k, double eps);
    void update(int oid, Position position, bool use_lazy = true);
    void print_result();
private:
    static constexpr ll DELIMITER_MIN = 2147483648LL;
    static std::mt19937 random_generator;
    struct PatternItem;
    struct Text;

    int N; // 全部text总长度
    map<int, Text> text_map; // oid -> Text
    unordered_map<int, pair<int, int>> name_map; // object_name -> (object_id, text_idx)
    unordered_map<ll, shared_ptr<PatternItem>> root_items;
    unordered_map<vector<int>, int, hash_vector> scratch_result;

    void increment_extend(OT insert_ot, PatternItem *ptr_item, Text *update_text, bool use_lazy);
    void static_extend(PatternItem *ptr_item, Text *update_text, bool use_lazy);
    void set_item_map(unordered_map<ll, shared_ptr<PatternItem>> &item_map, vector<int> &cluster, int depth, ll camera, Text *update_text, bool use_lazy);
    void update_result();
    void collect_patterns(map<int, vector<int>>::iterator crt_it, map<int, vector<int>>::iterator end_it, vector<int> &begins, vector<vector<int>> &begins_list);
};

std::mt19937 TreeMiner::random_generator(std::random_device{}());

struct TreeMiner::Text{
    int oid;
    reverse_vector<Position> positions;

    Text(int oid, vector<Position> vec) : oid(oid), positions(vec) {}
    explicit Text(Path &path) : oid(path.object_id), positions(path.positions) {}

    ll get_camera(int idx) const {return positions[idx].camera;}
    double get_time(int idx) const {return positions[idx].ts;}
    int size() const {return static_cast<int>(positions.size());}
};

struct TreeMiner::PatternItem{
    int depth;
    bool lazy_tag;

    OTMap sorted_objects;
    vector<vector<int>> clusters;
    vector<unordered_map<ll, shared_ptr<PatternItem>>> nxt_items;
    vector<OT> stub_objects;
    PatternItem(int depth, bool lazy_tag) : depth(depth), lazy_tag(lazy_tag) {}
};

TreeMiner::TreeMiner(int m, int k, double eps){
    N = 0;
    this->m = m;
    this->k = k;
    this->eps = eps;
}

void TreeMiner::update(int oid, Position position, bool use_lazy){
    Text *update_text = nullptr;
    auto map_itr = text_map.find(oid);
    if(map_itr == text_map.end()){
        ll delimiter = DELIMITER_MIN + static_cast<ll>(text_map.size());
        Text text(oid, {Position(delimiter, -1, -1)});
        auto indicator = text_map.insert(make_pair(oid, text));
        update_text = &(indicator.first->second);
    }
    else update_text = &map_itr->second;
    update_text->positions.push_front(position);
    N += 1;

    // 挖掘相关
    int name = static_cast<int>(name_map.size());
    int text_idx = update_text->size() * -1;
    name_map.insert(make_pair(name, make_pair(oid, text_idx)));

    ll root_camera = update_text->get_camera(0);
    shared_ptr<PatternItem> &ptr = root_items[root_camera];
    if(!ptr) ptr = make_shared<PatternItem>(1, false);
    increment_extend(OT(name, position.ts), ptr.get(), update_text, use_lazy);

    update_result();
    scratch_result = {};
}
int num_affected = 0;
void TreeMiner::increment_extend(OT insert_ot, PatternItem *ptr_item, Text *update_text, bool use_lazy){
    OTMap &sorted_objects = ptr_item->sorted_objects;
    vector<vector<int>> &clusters = ptr_item->clusters;
    vector<unordered_map<ll, shared_ptr<PatternItem>>> &nxt_items = ptr_item->nxt_items;

    PBIterator left_it = sorted_objects.lower_bound(insert_ot.begin_time - eps);
    PBIterator right_it = sorted_objects.upper_bound(insert_ot.begin_time + eps);
    if(left_it == sorted_objects.end() || right_it == sorted_objects.end() || right_it->first < left_it->first){
        int cid = static_cast<int>(clusters.size());
        clusters.push_back({insert_ot.object_name});
        num_affected += 1;

        ll nxt_camera = update_text->get_camera(ptr_item->depth);
        double nxt_time = update_text->get_time(ptr_item->depth);
        if(use_lazy){
            shared_ptr<PatternItem> nxt_item = make_shared<PatternItem>(ptr_item->depth + 1, true);
            nxt_item->stub_objects.emplace_back(insert_ot.object_name, nxt_time);
            nxt_items.push_back({{nxt_camera, nxt_item}});
        }
        else{
            shared_ptr<PatternItem> nxt_item = make_shared<PatternItem>(ptr_item->depth + 1, false);
            nxt_items.push_back({{nxt_camera, nxt_item}});
            if(nxt_camera < DELIMITER_MIN)
                increment_extend(OT(insert_ot.object_name, nxt_time), nxt_item.get(), update_text, use_lazy);
        }

        sorted_objects.insert(insert_ot, Border(cid, cid));
        return;
    }

    Border insert_border;
    vector<PBIterator> pbits;
    unordered_map<int, int> left_borders; // cid to offset
    int left_cid = -1, right_cid = -1;

    PBIterator end_it = std::next(right_it);
    int offset = 0;
    for(PBIterator tmp_it = left_it; tmp_it != end_it; ++tmp_it, ++offset){
        pbits.push_back(tmp_it);
        int left_border = tmp_it->second.left_border; // border == -1 表示不存在聚类边界
        int right_border = tmp_it->second.right_border;

        if(left_border != -1){
            if(tmp_it == left_it) left_cid = left_border;
            left_borders.insert(make_pair(left_border, offset));
        }
        if(right_border != -1){
            if(tmp_it == right_it) right_cid = right_border;
            auto border_it = left_borders.find(right_border);
            if(border_it != left_borders.end()){
                int target_cid = border_it->first;
                clusters[target_cid].push_back(insert_ot.object_name);
                num_affected += 1;
                if(target_cid == left_cid && tmp_it->first < insert_ot){
                    const_cast<Border&>(tmp_it->second).right_border = -1;
                    insert_border.right_border = target_cid;
                }
                if(target_cid == right_cid && insert_ot < pbits[border_it->second]->first){
                    const_cast<Border&>(pbits[border_it->second]->second).left_border = -1;
                    insert_border.left_border = target_cid;
                }
                // 尝试收集当前聚类所代表的pattern（扩增型）
                if(clusters[target_cid].size() >= m && ptr_item->depth >= k){
                    auto result_it = scratch_result.find(clusters[target_cid]);
                    if(result_it == scratch_result.end()) scratch_result.insert(make_pair(clusters[target_cid], ptr_item->depth));
                    else result_it->second = max(result_it->second, ptr_item->depth);
                }
                // 更新nxt_items
                unordered_map<ll, shared_ptr<PatternItem>> &item_map = nxt_items[target_cid];
                ll nxt_camera = update_text->get_camera(ptr_item->depth);
                double nxt_time = update_text->get_time(ptr_item->depth);
                auto item_it = item_map.find(nxt_camera);
                // 若当前聚类中的其他OT在经过下一个摄像头时均未与insert_ot出现在同一摄像头中
                if(item_it == item_map.end()){
                    if(use_lazy){
                        shared_ptr<PatternItem> nxt_item = make_shared<PatternItem>(ptr_item->depth + 1, true);
                        nxt_item->stub_objects.emplace_back(insert_ot.object_name, nxt_time);
                        item_map.insert(make_pair(nxt_camera, nxt_item));
                    }
                    else{
                        shared_ptr<PatternItem> nxt_item = make_shared<PatternItem>(ptr_item->depth + 1, false);
                        item_map.insert(make_pair(nxt_camera, nxt_item));
                        if(nxt_camera < DELIMITER_MIN)
                            increment_extend(OT(insert_ot.object_name, nxt_time), nxt_item.get(), update_text, use_lazy);
                    }
                }
                else{
                    const shared_ptr<PatternItem> &nxt_item = item_it->second;
                    // 若在下个摄像头上的PatternItem之前处在延迟更新状态
                    if(use_lazy && nxt_item->lazy_tag){
                        nxt_item->stub_objects.emplace_back(insert_ot.object_name, nxt_time);
                        if(nxt_item->stub_objects.size() >= m){
                            // 静态聚类
                            static_extend(nxt_item.get(), update_text, use_lazy);
                            nxt_item->lazy_tag = false;
                            nxt_item->stub_objects = {};
                        }
                    }
                    else increment_extend(OT(insert_ot.object_name, nxt_time), nxt_item.get(), update_text, use_lazy);
                }
            }
        }
    }
    // 检查当前PatternItem在insert_ot插入时能否产生新的cluster
    int target_offset = -1;
    if(left_cid == -1){
        vector<int> cluster;
        for(target_offset = 0; target_offset < pbits.size(); ++target_offset){
            PBIterator &tmp_it = pbits[target_offset];
            if(insert_ot < tmp_it->first) break;
            cluster.push_back(tmp_it->first.object_name);
        }
        if(!cluster.empty()){
            // 更新clusters
            cluster.push_back(insert_ot.object_name);
            int new_cid = static_cast<int>(clusters.size());
            clusters.push_back(cluster);
            num_affected += 1;
            const_cast<Border&>(left_it->second).left_border = new_cid;
            insert_border.right_border = new_cid;
            // 尝试收集当前聚类所代表的pattern（完全插入型）
            if(cluster.size() >= m && ptr_item->depth >= k){
                auto result_it = scratch_result.find(cluster);
                if(result_it == scratch_result.end()) scratch_result.insert(make_pair(cluster, ptr_item->depth));
                else result_it->second = max(result_it->second, ptr_item->depth);
            }
            // 更新nxt_items
            nxt_items.emplace_back();
            unordered_map<ll, shared_ptr<PatternItem>> &item_map = nxt_items.back();
            int nxt_depth = ptr_item->depth + 1;
            ll nxt_camera = update_text->get_camera(ptr_item->depth);
            set_item_map(item_map, cluster, nxt_depth, nxt_camera, update_text, use_lazy);
        }
    }
    if(right_cid == -1){
        vector<int> cluster;
        if(target_offset == -1){
            for(target_offset = static_cast<int>(pbits.size()) - 1; target_offset >= 0; --target_offset){
                PBIterator &tmp_it = pbits[target_offset];
                if(tmp_it->first < insert_ot) break;
                cluster.push_back(tmp_it->first.object_name);
            }
        }
        else{
            for(;target_offset < pbits.size(); ++target_offset){
                cluster.push_back(pbits[target_offset]->first.object_name);
            }
        }

        if(!cluster.empty()){
            // 更新clusters
            cluster.push_back(insert_ot.object_name);
            int new_cid = static_cast<int>(clusters.size());
            clusters.push_back(cluster);
            num_affected += 1;
            insert_border.left_border = new_cid;
            const_cast<Border&>(right_it->second).right_border = new_cid;
            // 尝试收集当前聚类所代表的pattern（完全插入型）
            if(cluster.size() >= m && ptr_item->depth >= k){
                auto result_it = scratch_result.find(cluster);
                if(result_it == scratch_result.end()) scratch_result.insert(make_pair(cluster, ptr_item->depth));
                else result_it->second = max(result_it->second, ptr_item->depth);
            }
            // 更新nxt_items
            nxt_items.emplace_back();
            unordered_map<ll, shared_ptr<PatternItem>> &item_map = nxt_items.back();
            int nxt_depth = ptr_item->depth + 1;
            ll nxt_camera = update_text->get_camera(ptr_item->depth);
            set_item_map(item_map, cluster, nxt_depth, nxt_camera, update_text, use_lazy);
        }
    }

    sorted_objects.insert(insert_ot, insert_border);
}

void TreeMiner::static_extend(PatternItem *ptr_item, Text *update_text, bool use_lazy){
    vector<OT> &ots = ptr_item->stub_objects;
    vector<vector<int>> &clusters = ptr_item->clusters;
    OTMap &sorted_objects = ptr_item->sorted_objects;
    vector<unordered_map<ll, shared_ptr<PatternItem>>> &nxt_items = ptr_item->nxt_items;

    int size = static_cast<int>(ots.size());
    sort(ots.begin(), ots.end());
    vector<Border> borders(size, Border());

    int left_idx = 0, right_idx = 0;
    deque<int> queue{ots.front().object_name};
    double limit = ots.front().begin_time + eps;
    while(right_idx < size - 1){
        int nxt_idx = right_idx + 1;
        double nxt_time = ots[nxt_idx].begin_time;
        if(nxt_time <= limit){
            right_idx++;
            queue.push_back(ots[right_idx].object_name);
        }
        else{
            // 更新clusters
            int cid = static_cast<int>(clusters.size());
            clusters.emplace_back(queue.begin(), queue.end());
            borders[left_idx].left_border = cid;
            borders[right_idx].right_border = cid;
            // 尝试收集当前聚类所代表的pattern（完全插入型）
            vector<int> &cluster = clusters.back();
            if(cluster.size() >= m && ptr_item->depth >= k){
                auto result_it = scratch_result.find(cluster);
                if(result_it == scratch_result.end()) scratch_result.insert(make_pair(cluster, ptr_item->depth));
                else result_it->second = max(result_it->second, ptr_item->depth);
            }
            // 更新nxt_items
            nxt_items.emplace_back();
            unordered_map<ll, shared_ptr<PatternItem>> &item_map = nxt_items.back();
            int nxt_depth = ptr_item->depth + 1;
            ll nxt_camera = update_text->get_camera(ptr_item->depth);
            set_item_map(item_map, cluster, nxt_depth, nxt_camera, update_text, use_lazy);

            while(left_idx != nxt_idx){
                if(nxt_time - ots[left_idx].begin_time <= eps) break;
                else{
                    left_idx++;
                    queue.pop_front();
                }
            }
            limit = ots[left_idx].begin_time + eps;
            right_idx++;
            queue.push_back(ots[right_idx].object_name);
        }
    }
    // 最后一次更新clusters
    int cid = static_cast<int>(clusters.size());
    clusters.emplace_back(queue.begin(), queue.end());
    num_affected += static_cast<int>(clusters.size());
    borders[left_idx].left_border = cid;
    borders[right_idx].right_border = cid;
    // 尝试收集当前聚类所代表的pattern（完全插入型）
    vector<int> &cluster = clusters.back();
    if(cluster.size() >= m && ptr_item->depth >= k){
        auto result_it = scratch_result.find(cluster);
        if(result_it == scratch_result.end()) scratch_result.insert(make_pair(cluster, ptr_item->depth));
        else result_it->second = max(result_it->second, ptr_item->depth);
    }
    // 更新nxt_items
    nxt_items.emplace_back();
    unordered_map<ll, shared_ptr<PatternItem>> &item_map = nxt_items.back();
    int nxt_depth = ptr_item->depth + 1;
    ll nxt_camera = update_text->get_camera(ptr_item->depth);
    set_item_map(item_map, cluster, nxt_depth, nxt_camera, update_text, use_lazy);

    // 为了避免插入有序元素影响顺序，以打乱的顺序构造sorted_objects
    vector<int> indices(size);
    for(int i = 0; i < size; ++i) indices[i] = i;
    shuffle(indices.begin(), indices.end(), random_generator);
    for(int i = 0; i < size; ++i) {
        int idx = indices[i];
        sorted_objects.insert(ots[idx], borders[idx]);
    }
}

void TreeMiner::set_item_map(unordered_map<ll, shared_ptr<PatternItem>> &item_map, vector<int> &cluster, int depth, ll camera, Text *update_text, bool use_lazy){
    unordered_map<ll, vector<OT>> stub_map;
    for(int crt_name : cluster){
        const pair<int, int> &id_entry = name_map.at(crt_name);
        int oid = id_entry.first;
        int tid = id_entry.second + (depth - 1);
        const Text &crt_text = text_map.at(oid);
        ll crt_camera = crt_text.get_camera(tid);
        double crt_time = crt_text.get_time(tid);
        stub_map[crt_camera].emplace_back(crt_name, crt_time);
    }
    for(auto &stub_entry : stub_map){
        ll crt_camera = stub_entry.first;
        const vector<OT> &crt_ots = stub_entry.second;
        shared_ptr<PatternItem> crt_item = make_shared<PatternItem>(depth, true);
        crt_item->stub_objects = crt_ots;
        item_map.insert(make_pair(crt_camera, crt_item));
        if(use_lazy){
            if(crt_camera == camera && crt_ots.size() >= m){
                // 静态聚类
                static_extend(crt_item.get(), update_text, use_lazy);
                crt_item->lazy_tag = false;
                crt_item->stub_objects = {};
            }
        }
        else{
            if(crt_camera == camera){
                if(crt_camera < DELIMITER_MIN) static_extend(crt_item.get(), update_text, use_lazy); // 静态聚类
                crt_item->lazy_tag = false;
                crt_item->stub_objects = {};
            }
        }
    }
}

void TreeMiner::update_result(){
    for(const auto &entry : scratch_result){
        const vector<int> &names = entry.first;
        int depth = entry.second;
        map<int, vector<int>> oid_to_begins;
        for(const int &name : names){
            const pair<int, int> &id_pair = name_map.at(name);
            oid_to_begins[id_pair.first].push_back(id_pair.second);
        }
        if(oid_to_begins.size() < m) return;

        vector<int> oids;
        list<vector<pair<int, int>>> trajs;
        for(const auto &oid_entry : oid_to_begins) oids.push_back(oid_entry.first);
        if(oids.size() == names.size()){
            trajs.emplace_back();
            vector<pair<int, int>> &traj = trajs.back();
            for(const auto &oid_entry : oid_to_begins){
                int begin = oid_entry.second.front(), end = begin + depth - 1;
                traj.emplace_back(begin, end);
            }
        }
        else{
            vector<int> tmp_begins; vector<vector<int>> begins_list;
            collect_patterns(oid_to_begins.begin(), oid_to_begins.end(), tmp_begins, begins_list);
            for(const vector<int> &begins : begins_list){
                trajs.emplace_back();
                vector<pair<int, int>> &traj = trajs.back();
                for(const int &begin : begins) traj.emplace_back(begin, begin + depth - 1);
            }
        }

        result.insert(oids, trajs);
    }
}

void TreeMiner::collect_patterns(map<int, vector<int>>::iterator crt_it, map<int, vector<int>>::iterator end_it, vector<int> &begins, vector<vector<int>> &begins_list){
    if(crt_it == end_it){
        begins_list.push_back(begins);
        return;
    }
    for(const int &begin : crt_it->second){
        begins.push_back(begin);
        collect_patterns(next(crt_it), end_it, begins, begins_list);
        begins.pop_back();
    }
}

void TreeMiner::print_result(){
    int n_objects = 0, n_convoys = 0;
    for(const auto &entry : result.result_map){
        if(entry.second.empty()) continue;
        n_objects++;
        n_convoys += static_cast<int>(entry.second.size());
    }
    cout << "Total number of discovered convoys: " << n_convoys << ", object combinations: " << n_objects << endl;
//    for(const auto &entry : result.result_map){
//        const vector<int> &oids = entry.first;
//        const list<vector<pair<int, int>>> &trajs = entry.second;
//        cout<<"{";
//        for(int oid : oids){
//            if(oid != oids.back()) cout<<oid<<",";
//            else cout<<oid;
//        }
//        cout<<"} : [";
//        if(!trajs.empty()){
//            for(const vector<pair<int, int>> &traj : trajs){
//                int crt_oid = oids.front();
//                const pair<int, int> &interval = traj.front();
//                Text &text = text_map.at(crt_oid);
//                cout<<"(";
//                for(int tmp = interval.second; tmp >= interval.first; --tmp){
//                    if(tmp == interval.first) cout<<text.get_camera(tmp);
//                    else cout<<text.get_camera(tmp)<<"-";
//                }
//                cout<<"), ";
//            }
//        }
//        cout<<"]"<<endl;
//    }
}

#endif //EXPERIMENT_MINER_H

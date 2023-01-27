#ifndef PROJECT_TREE_H
#define PROJECT_TREE_H

#include <unordered_map>
#include <vector>
#include <map>
#include <climits>
#include <iostream>
#include "position.h"

using namespace std;
typedef long long ll;

struct MiningNode{
    ll start, end, slink;
    map<position, ll> next;
    bool is_leaf;
    ll depth;
    ll leaf_left, leaf_right;

    ll edge_length() const {
        return end - start;
    }
};

class MiningTree{
private:
    ll text_size;
    ll last_added, pos, needSL, remainder, active_node, active_e, active_len;
    vector<vector<position>>& positions_list;

    ll new_node(ll start, ll end);
    position& active_edge() const;
    void add_SL(ll node_id);
    bool walk_down(ll node_id);
    void st_extend(const position& element);
    pair<ll, ll> save_leaves(ll node_id);
public:
    ll root;
    MiningNode *tree;
    position *text;
    ll *to_terminator;
    unordered_map<ll, int> terminator2ListId;
    vector<ll> leaves;

    explicit MiningTree(vector<vector<position>>& positions_list);
    ~MiningTree(){
        delete[] tree, text, to_terminator;
    }
};

MiningTree::MiningTree(vector<vector<position>>& positions_list) : positions_list(positions_list){
    int n_positions = positions_list.size();

    text_size = 0;
    ll terminator = ll(INT_MAX) + 1;
    ll *terminatorPos = new ll[n_positions];
    for(int i = 0;i < n_positions;i++){
        positions_list[i].push_back(position(terminator));
        text_size += positions_list[i].size();
        terminatorPos[i] = text_size - 1;
        terminator2ListId[terminator] = i;
        terminator++;
    }
    to_terminator = new ll[text_size];
    ll cur_terminator_id = 0;
    for(ll i = 0; i < text_size; i++){
        to_terminator[i] = terminatorPos[cur_terminator_id] - i;
        if(to_terminator[i] == 0) cur_terminator_id++;
    }
    delete[] terminatorPos;

    //基本参数初始化
    tree = new MiningNode[text_size * 2];
    text = new position[text_size];
    needSL = 0, last_added = 0, pos = -1, remainder = 0, active_node = 0, active_e = 0, active_len = 0;
    root = active_node = new_node(-1, -1);
    tree[root].is_leaf = false;// 默认根节点不是叶节点
    tree[root].depth = 0;
    //开始逐个构建
    for(int i = 0;i < n_positions;i++){
        vector<position>& positions = positions_list[i];
        for(position& element : positions) st_extend(element);
    }
    save_leaves(root);
}

ll MiningTree::new_node(ll start, ll end) {
    MiningNode node;
    node.start = start;
    node.end = end;
    node.slink = 0;
    tree[++last_added] = node;
    return last_added;
}

position& MiningTree::active_edge() const {
    return text[active_e];
}

void MiningTree::add_SL(ll node_id) {
    if (needSL > 0) tree[needSL].slink = node_id;
    needSL = node_id;
}

bool MiningTree::walk_down(ll node_id) {
    if (active_len >= tree[node_id].edge_length()) {
        active_e += tree[node_id].edge_length();
        active_len -= tree[node_id].edge_length();
        active_node = node_id;
        return true;
    }
    return false;
}

void MiningTree::st_extend(const position& element) {
    text[++pos] = element;
    needSL = 0;
    remainder++;
    while(remainder > 0){
        if(active_len == 0) active_e = pos;
        if(tree[active_node].next.find(active_edge()) == tree[active_node].next.end()){
            ll leaf = new_node(pos, pos + to_terminator[pos] + 1);
            tree[leaf].is_leaf = true;
            tree[leaf].depth = tree[active_node].depth + tree[leaf].edge_length();
            tree[active_node].next[active_edge()] = leaf;
            add_SL(active_node);
        }
        else{
            ll nxt = tree[active_node].next[active_edge()];
            if(walk_down(nxt)) continue;
            if(text[tree[nxt].start + active_len] == element){
                active_len++;
                add_SL(active_node);
                break;
            }
            ll split = new_node(tree[nxt].start, tree[nxt].start + active_len);
            tree[split].is_leaf = false;
            tree[split].depth = tree[active_node].depth + tree[split].edge_length();
            tree[active_node].next[active_edge()] = split;
            ll leaf = new_node(pos, pos + to_terminator[pos] + 1);
            tree[leaf].is_leaf = true;
            tree[leaf].depth = tree[split].depth + tree[leaf].edge_length();
            tree[split].next[element] = leaf;
            tree[nxt].start += active_len;
            tree[split].next[text[tree[nxt].start]] = nxt;
            add_SL(split);
        }
        remainder--;
        if (active_node == root && active_len > 0){
            active_len--;
            active_e = pos - remainder + 1;
        }
        else active_node = tree[active_node].slink > 0 ? tree[active_node].slink : root;
    }
}

pair<ll, ll> MiningTree::save_leaves(ll node_id) {
    if(tree[node_id].is_leaf){
        leaves.emplace_back(node_id);
        tree[node_id].leaf_left = leaves.size() - 1;
        tree[node_id].leaf_right = leaves.size() - 1;
        return make_pair(tree[node_id].leaf_left, tree[node_id].leaf_right);
    }
    bool first_visit = true;
    pair<ll, ll> range, cur_range;
    for(auto& entry : tree[node_id].next){
        ll nxt = entry.second;
        cur_range = save_leaves(nxt);
        if(first_visit){
            first_visit = false;
            range.first = cur_range.first;
        }
    }
    range.second = cur_range.second;
    tree[node_id].leaf_left = range.first;
    tree[node_id].leaf_right = range.second;
    return range;
}

#endif //PROJECT_TREE_H

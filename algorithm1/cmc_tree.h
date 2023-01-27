#ifndef PROJECT_CMC_TREE_H
#define PROJECT_CMC_TREE_H
#include <iostream>
#include <climits>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <map>

using namespace std;
typedef long long ll;
struct Node {
    ll start, end, slink;
    map<ll, ll> next;//从 边上的首字符 映射到 节点在tree中的位置
    bool is_leaf;
    ll depth;//节点深度
    ll leaf_left, leaf_right;//以此节点为根的子树的全部叶节点的左右边界标号

    ll edge_length() const {
        return end - start;
    }
};

class SuffixTree{
private:
    ll last_added, pos, needSL, remainder, active_node, active_e, active_len;
    ll text_size;
    vector<ll> *arr_string;//需要添加到后缀树中的串数组
    vector<pair<double, double>> *arr_interval;//与串数组对应，一个串中的元素对应一个时间区间
    int arr_length;//串的数量
    bool arr_delete = false;//arr_string是否需要在析构函数中被删除
    unordered_map<ll, int> terminator2StringId;//记录不同串的终止符在串数组中的位置
    ll *toTerminator;//记录任意串中某字符到属于该串的终止符的距离

    void st_init(vector<ll>*, vector<pair<double, double>>*, int);
    ll new_node(ll, ll);
    ll active_edge() const;
    void add_SL(ll);
    bool walk_down(ll);
    void st_extend(ll);
    pair<ll, ll> save_leaves(ll node_id);
    void printPath(ll, vector<ll>);
public:
    ll root;
    Node *tree;
    ll *text;
    pair<double, double> *time;
    vector<ll> leaves;

    SuffixTree(string *strings, vector<pair<double, double>> *intervals, int n_strings): arr_length(n_strings){
        arr_delete = true;
        arr_string = new vector<ll>[arr_length];
        for(int i = 0;i < arr_length;i++){
            vector<ll> list(strings[i].begin(), strings[i].end());
            arr_string[i] = list;
        }
        st_init(arr_string, intervals, arr_length);
    }
    SuffixTree(vector<ll> *strings, vector<pair<double, double>> *intervals, int n_strings){
        st_init(strings, intervals, n_strings);
    }
    ~SuffixTree(){
        delete[] toTerminator, text, tree, time;
        if(arr_delete) delete[] arr_string;
    }

    int terminator2id(ll terminator){return terminator2StringId[terminator];}
    void printPath() {
        printPath(1, vector<ll>());
    }
};

void SuffixTree::st_init(vector<ll> *strings, vector<pair<double, double>> *intervals, int n_strings) {
    arr_string = strings;
    arr_interval = intervals;
    arr_length = n_strings;
    text_size = 0;
    ll terminator = ll(INT_MAX) + 1;
    ll *terminatorPos = new ll[arr_length];
    for(int i = 0;i < arr_length;i++){
        arr_string[i].push_back(terminator);
        text_size += arr_string[i].size();
        terminatorPos[i] = text_size - 1;
        terminator2StringId[terminator] = i;
        terminator++;
    }
    toTerminator = new ll[text_size];
    ll currTerminatorId = 0;
    for(ll i = 0; i < text_size; i++){
        toTerminator[i] = terminatorPos[currTerminatorId] - i;
        if(toTerminator[i] == 0)currTerminatorId++;
    }
    delete[] terminatorPos;
    //基本参数初始化
    tree = new Node[text_size * 2];
    text = new ll[text_size];
    time = new pair<double, double>[text_size];
    needSL = 0, last_added = 0, pos = -1, remainder = 0, active_node = 0, active_e = 0, active_len = 0;
    root = active_node = new_node(-1, -1);
    tree[root].is_leaf = false;//默认根节点不是叶节点
    tree[root].depth = 0;
    //开始逐个构建
    ll intervalPos = -1;
    for(int i = 0;i < arr_length;i++){
        vector<pair<double, double>> & pairs = arr_interval[i];
        for(pair<double, double> & interval : pairs)time[++intervalPos] = interval;
        time[++intervalPos] = make_pair(-1, -1);
    }
    for(int i = 0;i < arr_length;i++){
        vector<ll>& s = arr_string[i];
        for(ll element : s) st_extend(element);
    }
    save_leaves(root);
}

ll SuffixTree::new_node(ll start, ll end) {
	Node node;
	node.start = start;
	node.end = end;
    node.slink = 0;
	tree[++last_added] = node;
	return last_added;
}

ll SuffixTree::active_edge() const {
	return text[active_e];
}

void SuffixTree::add_SL(ll node_id) {
	if (needSL > 0) tree[needSL].slink = node_id;
	needSL = node_id;
}

bool SuffixTree::walk_down(ll node_id) {
	if (active_len >= tree[node_id].edge_length()) {
		active_e += tree[node_id].edge_length();
		active_len -= tree[node_id].edge_length();
		active_node = node_id;
		return true;
	}
	return false;
}

void SuffixTree::st_extend(ll element) {
	text[++pos] = element;
	needSL = 0;
	remainder++;
	while(remainder > 0) {
		if (active_len == 0) active_e = pos;
		if (tree[active_node].next.find(active_edge()) == tree[active_node].next.end()) {//若没有对应边
			ll leaf = new_node(pos, pos + toTerminator[pos] + 1);
            tree[leaf].is_leaf = true;
            tree[leaf].depth = tree[active_node].depth + tree[leaf].edge_length();
            tree[active_node].next[active_edge()] = leaf;
			add_SL(active_node); //rule 2
		} else {
			ll nxt = tree[active_node].next[active_edge()];
			if (walk_down(nxt)) continue; //observation 2
			if (text[tree[nxt].start + active_len] == element) { //observation 1
				active_len++;
				add_SL(active_node); //observation 3
				break;
			}
			ll split = new_node(tree[nxt].start, tree[nxt].start + active_len);
            tree[split].is_leaf = false;
            tree[split].depth = tree[active_node].depth + tree[split].edge_length();
			tree[active_node].next[active_edge()] = split;
			ll leaf = new_node(pos, pos + toTerminator[pos] + 1);
            tree[leaf].is_leaf = true;
            tree[leaf].depth = tree[split].depth + tree[leaf].edge_length();
			tree[split].next[element] = leaf;
			tree[nxt].start += active_len;
			tree[split].next[text[tree[nxt].start]] = nxt;
			add_SL(split); //rule 2
		}
		remainder--;
		if (active_node == root && active_len > 0) { //rule 1
			active_len--;
			active_e = pos - remainder + 1;
		} else
			active_node = tree[active_node].slink > 0 ? tree[active_node].slink : root; //rule 3
	}
}

pair<ll, ll> SuffixTree::save_leaves(ll node_id) {
    if(tree[node_id].is_leaf){
        leaves.emplace_back(node_id);
        tree[node_id].leaf_left = leaves.size() - 1;
        tree[node_id].leaf_right = leaves.size() - 1;
        return make_pair(tree[node_id].leaf_left, tree[node_id].leaf_right);
    }
    bool first_visit = true;
    pair<ll, ll> range, cur_range;
    for(auto & it : tree[node_id].next){
        ll nxt = it.second;
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

void SuffixTree::printPath(ll node_id, vector<ll> ansList){
    bool isLeaf = true;
    for(auto & it : tree[node_id].next){
        isLeaf = false;
        ll nxt=it.second;
        vector<ll> tmpList(ansList);
        for(int i = tree[nxt].start;i < tree[nxt].start+tree[nxt].edge_length();i++){
            tmpList.push_back(text[i]);
        }
        printPath(nxt, tmpList);
    }
    if(isLeaf){
        for(ll l : ansList){
            cout<<l<<",";
        }
        cout<<endl;
    }
}

#endif //PROJECT_CMC_TREE_H
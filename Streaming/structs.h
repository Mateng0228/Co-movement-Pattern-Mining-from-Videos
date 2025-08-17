#ifndef EXPERIMENT_STRUCTS_H
#define EXPERIMENT_STRUCTS_H

#include <string>
#include <vector>
#include <list>
#include <unordered_map>
#include <cmath>
#include <ext/pb_ds/assoc_container.hpp>
#include <ext/pb_ds/tree_policy.hpp>
#include <cfloat>

typedef long long ll;
const double CP_DELTA = 1e-9;

using namespace __gnu_pbds;
using namespace std;

struct Position{
    ll camera = -1;
    double ts = -1, te = -1;

    explicit Position(ll camera, double start_time, double end_time){
        this->camera = camera;
        this->ts = start_time;
        this->te = end_time;
    }

    bool operator==(const Position &other) const {
        return camera == other.camera;
    }
};

struct Path{
    int object_id;
    std::vector<Position> positions;

    Path(int id, std::vector<Position> positions){
        this->object_id = id;
        this->positions = std::move(positions);
    }
};


template <typename T>
class reverse_vector{
private:
    int n;
    std::vector<T> vec;
public:
    reverse_vector() : n(0), vec() {}
    explicit reverse_vector(std::vector<T> &v) : n(v.size()), vec(v) {}
    reverse_vector(const reverse_vector &other): n(other.n), vec(other.vec) {}

    template <typename... Args>
    void emplace_front(Args&&... args){
        vec.emplace_back(std::forward<Args>(args)...);
        n += 1;
    }

    void push_front(const T &item){
        vec.push_back(item);
        n += 1;
    }

    T& front(){
        if(n == 0) throw std::out_of_range("Reverse vector is empty");
        return vec.back();
    }

    void pop_front(){
        if(n == 0) throw std::out_of_range("Reverse vector is empty");
        vec.pop_back();
        n -= 1;
    }

    T& back(){
        if(n == 0) throw std::out_of_range("Reverse vector is empty");
        return vec.front();
    }

    T& operator[](int i){
        if (i >= n || i < -static_cast<int>(n)) throw std::out_of_range("Index out of range");
        if (i < 0) return vec[-1 - i];
        return vec[n - 1 - i];
    }

    const T& operator[](int i) const {
        if (i >= n || i < -static_cast<int>(n)) throw std::out_of_range("Index out of range");
        if (i < 0) return vec[-1 - i];
        return vec[n - 1 - i];
    }

    using iterator = std::reverse_iterator<typename std::vector<T>::iterator>;
    using const_iterator = std::reverse_iterator<typename std::vector<T>::const_iterator>;

    iterator begin(){
        return vec.rbegin();
    }
    const_iterator begin() const {
        return vec.crbegin();
    }
    const_iterator cbegin() const {
        return vec.crbegin();
    }

    iterator end(){
        return vec.rend();
    }
    const_iterator end() const{
        return vec.crend();
    }
    const_iterator cend() const{
        return vec.crend();
    }


    size_t size() const {
        return n;
    }
};

struct hash_vector {
    inline std::size_t operator()(const std::vector<int>& vec) const {
        std::size_t seed = vec.size();
        for (int val : vec) {
            seed ^= static_cast<std::size_t>(val)
                    + 0x9e3779b9
                    + (seed << 6)
                    + (seed >> 2);
        }
        return seed;
    }
};

struct OT{
    int object_name;
    double begin_time;

    OT(int name, double time) : object_name(name), begin_time(time) {};
    bool operator<(const OT &other) const {
        if(std::fabs(begin_time - other.begin_time) > CP_DELTA) return begin_time < other.begin_time;
        return object_name < other.object_name;
    }
    friend std::ostream& operator<<(std::ostream& os, const OT& ot) {
        return os << "[object: " << ot.object_name << ", time: " << ot.begin_time << "]";
    }
};
struct Border{
    int left_border;
    int right_border;
    Border() : left_border(-1), right_border(-1) {}
    Border(int left, int right) : left_border(left), right_border(right) {}
    friend std::ostream& operator<<(std::ostream& os, const Border& bd) {
        return os << "[" << bd.left_border << ", " << bd.right_border<< "]";
    }
};


using PBMap = tree<OT, Border, std::less<>, rb_tree_tag, tree_order_statistics_node_update>;
using PBIterator = PBMap::const_iterator;
class OTMap{
private:
    PBMap ps;

public:


    void insert(const OT &ot, const Border &border) {
        PBIterator crt_it = ps.insert({ot, border}).first;
    }

    // 返回 begin_time >= val 的最小迭代器
    PBIterator lower_bound(double val) {
        return ps.lower_bound(OT{-1, val});
    }

    // 返回 begin_time <= val 的最大迭代器
    PBIterator upper_bound(double val) {
//        auto it = ps.upper_bound(OT{-1, std::nextafter(val, DBL_MAX)});
        auto it = ps.upper_bound(OT{INT32_MAX, val});
        if (it == ps.begin()) return ps.end();
        return --it;
    }

    Border& at(int index){
        return ps.find_by_order(index)->second;
    }

    PBIterator begin(){return ps.begin();}
    PBIterator end(){return ps.end();}
    int size() const {return static_cast<int>(ps.size());}
};



#endif //EXPERIMENT_STRUCTS_H

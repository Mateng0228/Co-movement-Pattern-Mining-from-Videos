#ifndef PROJECT_POSITION_H
#define PROJECT_POSITION_H

#include <vector>

struct offline_position{
    long long camera;
    double interval_left, interval_right;
    int group;
    std::vector<int> clusters;

    offline_position(){
        camera = -1;
        interval_left = -1;
        interval_right = -1;
        group = -1;
    }

    explicit offline_position(long long camera, double interval_left = -1, double interval_right = -1, int group = -1){
        this->camera = camera;
        this->interval_left = interval_left;
        this->interval_right = interval_right;
        this->group = group;
    }

    bool operator<(const offline_position& pos2) const {
        if(camera == pos2.camera) return group < pos2.group;
        return camera < pos2.camera;
    }

    bool operator==(const offline_position& pos2) const{
        return camera == pos2.camera && group == pos2.group;
    }
};

#endif //PROJECT_POSITION_H

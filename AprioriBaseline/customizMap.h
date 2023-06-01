//
// Created by lenovo on 2022/12/20.
//

#ifndef CONVOYBASELINE_CUSTOMIZMAP_H
#define CONVOYBASELINE_CUSTOMIZMAP_H

#endif //CONVOYBASELINE_CUSTOMIZMAP_H
#include "HeadFiles.h"

// 自定义map中的key值判断
struct cmp_key{
    bool operator()(const vector<ll> &k1, const vector<ll> &k2)const{
        if(k1.size()!=k2.size()){
            return k1.size()<k2.size();
        }
        for(ll i=0;i<k1.size();i++){
            if(k1[i]!=k2[i]){
                return k1[i]<k2[i];
            }
        }
        return false;
    }
};


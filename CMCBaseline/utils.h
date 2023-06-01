//
// Created by lenovo on 2023/4/11.
//

#ifndef CMC_UTILS_H
#define CMC_UTILS_H

#endif //CMC_UTILS_H

#include <iostream>
#include <string.h>
#include <vector>
#include <sys/types.h>
#include <dirent.h>
#include <fstream>
#include <cmath>
#include <map>
#include <algorithm>
#include <deque>
#include <map>
#include <set>
#include <list>
#include <cmath>
#include <algorithm>

typedef long long ll;
using namespace std;

ll candidate_num;

/**
 * objId: 车辆Id
 * cameraId: 摄像头Id
 * startTime: 初始时间
 * offset: 偏移量
 */
typedef struct {
    ll objId, cameraId, startTime, offset;
} CameraInfo;

// 在每个摄像头id下根据时间eps滑动窗口时需要进行的排序
bool cmpByTime(CameraInfo &c1, CameraInfo &c2) {
    if (c1.startTime != c2.startTime) {
        return c1.startTime < c2.startTime;
    } else if (c1.objId != c2.objId) {
        return c1.objId < c2.objId;
    } else {
        return c1.offset < c2.offset;
    }
}

// 在每个摄像头id下根据时间eps滑动窗口时需要进行的排序
bool cmpStamp(vector<CameraInfo> &c1, vector<CameraInfo> &c2) {
    return c1[0].startTime < c2[0].startTime;
}

// 再次根据车辆id进行排序
bool cmpCar(CameraInfo &c1, CameraInfo &c2) {
    if (c1.objId != c2.objId) {
        return c1.objId < c2.objId;
    } else {
        return c1.offset < c2.offset;
    }
}

// 自定义map中的key值判断
struct mapKey {
    bool operator()(const vector<ll> &k1, const vector<ll> &k2) const {
        if (k1.size() != k2.size()) {
            return k1.size() < k2.size();
        }
        for (ll i = 0; i < k1.size(); i++) {
            if (k1[i] != k2[i]) {
                return k1[i] < k2[i];
            }
        }
        return false;
    }
};

struct mapCameraInfo {
    bool operator()(const vector<CameraInfo> &k1, const vector<CameraInfo> &k2) const {
        if (k1.size() != k2.size()) {
            return k1.size() < k2.size();
        }
        for (ll i = 0; i < k1.size(); i++) {
            if (k1[i].objId != k2[i].objId) {
                return k1[i].objId < k2[i].objId;
            }
            if (k1[i].offset != k2[i].offset) {
                return k1[i].offset < k2[i].offset;
            }
        }
        return false;
    }
};


// 自定义set中的key值判断
struct setKey {
    bool operator()(const vector<ll> &k1, const vector<ll> &k2) const {
        if (k1.size() != k2.size()) {
            return k1.size() < k2.size();
        }
        for (ll i = 0; i < k1.size(); i++) {
            if (k1[i]!= k2[i]) {
                return k1[i] < k2[i];
            }
        }
        return false;
    }
};

// 自定义set中的key值判断
struct setKeyInCamera {
    bool operator()(const vector<CameraInfo> &k1, const vector<CameraInfo> &k2) const {
        if (k1.size() != k2.size()) {
            return k1.size() < k2.size();
        }
        for (ll i = 0; i < k1.size(); i++) {
            if (k1[i].objId != k2[i].objId) {
                return k1[i].objId < k2[i].objId;
            } else if (k1[i].offset != k2[i].offset) {
                return k1[i].offset < k2[i].offset;
            } else {
                continue;
            }
        }
        return false;
    }
};

void dupConvoyInSameCameraList(map<vector<ll>, list<vector<CameraInfo>>, mapKey> &convoyPattern);

/**
 * 获取文件对应车辆ID
 * @param s
 * @return
 */
ll getId(string s) {
    ll pos = 0;
    ll j = 0;
    for (ll i = s.size() - 5; i >= 0; i--) {
        if (s[i] >= '0' && s[i] <= '9') {
            pos += pow(10, j) * (s[i] - '0');
            j++;
        } else break;
    }
    return pos;
}

/**
 * 获取某个文件夹下的所有文件路径
 * @param path
 * @param fileNames
 */
void getFileNames(string &path, vector<string> &fileNames) {
    DIR *pDir;
    struct dirent *ptr;
    if (!(pDir = opendir(path.c_str())))
        return;
    while ((ptr = readdir(pDir)) != 0) {
        if (strcmp(ptr->d_name, ".") != 0 && strcmp(ptr->d_name, "..") != 0)
            fileNames.push_back(path + "/" + ptr->d_name);
    }
    closedir(pDir);
}

/**
 * 根据文件获取各个Obj的路径信息
 * @param objPath
 * @param fileNames
 * @param objId
 * @param timePath
 */
void getDataByFile(map<ll, vector<CameraInfo>> &singleCamera, vector<CameraInfo> &objPath, vector<string> &fileNames,
                   map<ll, vector<ll>> &linkCamera) {

    clock_t beginTime, endTime;
    beginTime = clock();

    for (ll i = 0; i < fileNames.size(); i++) {
        CameraInfo cameraInfo;

        string file = fileNames[i];
        cameraInfo.objId = getId(file);
        // 文件流输入
        ifstream fin;
        string buffer;
        fin.open(file, ios::in);
        ll offset = 0;
        ll preCamera = 0;
        while (getline(fin, buffer)) {
            cameraInfo.offset = offset++;
            int ans = 0;
            int prefix = 1;
            ll num = 0;
            for (char c: buffer) {
                if (c == ',' || c == '\r') {
                    if (ans == 0) {
                        cameraInfo.cameraId = prefix * num;
                        // 判断连接摄像头ID
                        if (cameraInfo.offset) {
                            linkCamera[preCamera].push_back(cameraInfo.cameraId);
                        }
                        preCamera = cameraInfo.cameraId;
                    } else if (ans == 1) {
                        cameraInfo.startTime = prefix * num;
                    } else {
                        break;
                    }
                    ans++;
                    prefix = 1;
                    num = 0;
                } else if (c == '-') {
                    prefix = -1;
                } else {
                    num = 10 * num + c - '0';
                }
            }
            objPath.push_back(cameraInfo);
            singleCamera[cameraInfo.cameraId].push_back(cameraInfo);
        }
    }

    cout << "objPath size = " << objPath.size() << endl;
    cout << "singleCamera size = " << singleCamera.size() << endl;
    endTime = clock();
    printf("reading time : %lf(s)\n", (double) (endTime - beginTime) / CLOCKS_PER_SEC);
    return;
}


void getClusterByEps(map<ll, vector<CameraInfo>> &singleCamera, ll m, ll eps,
                     vector<vector<CameraInfo>> &singleCameraInEps) {


    clock_t beginTime, endTime;
    beginTime = clock();

    for (auto it = singleCamera.begin(); it != singleCamera.end(); it++) {
        vector<CameraInfo> objSet = it->second;
        // 对单个摄像头下的车辆路径信息根据进入时间排序
        sort(objSet.begin(), objSet.end(), cmpByTime);
        ll left, right = 0;

        // 候选集合，采用双端队列，适合对首尾元素进行删除
        deque<CameraInfo> objDeque;

        // 表示每个车辆的出现次数，为了统计之后是否大于等于m
        map<ll, ll> objOccur;

        // 修改部分
        for (left = 0; left < objSet.size(); left++) {
            bool isChanged = true;
            //bool isChanged = false;
            if (left) {
                objDeque.pop_front();
                objOccur[objSet[left - 1].objId]--;
                if (objOccur[objSet[left - 1].objId] == 0) {
                    objOccur.erase(objSet[left - 1].objId);
                }
            }
            while (right != objSet.size() && objSet[right].startTime - objSet[left].startTime <= eps) {
                objOccur[objSet[right].objId]++;
                objDeque.push_back(objSet[right]);
                isChanged = true;
                right++;
            }
            if (isChanged && objOccur.size() >= m) {
                vector<CameraInfo> insertObjSet;
                deque<CameraInfo> tmpDeq = objDeque;
                while (tmpDeq.size()) {
                    insertObjSet.push_back(tmpDeq.front());
                    tmpDeq.pop_front();
                }
                singleCameraInEps.push_back(insertObjSet);
            }
        }
    }

    sort(singleCameraInEps.begin(), singleCameraInEps.end(), cmpStamp);

    for (vector<CameraInfo> &objSet: singleCameraInEps) {
        sort(objSet.begin(), objSet.end(), cmpCar);
    }

    printf("time stamp size: %ld\n", singleCameraInEps.size());

    endTime = clock();
    printf("clustering time : %lf(s)\n", (double) (endTime - beginTime) / CLOCKS_PER_SEC);
    return;
}


// 改进cmc算法
void cmc(vector<vector<CameraInfo>> &singleCameraInEps,
         map<vector<ll>, list<vector<CameraInfo>>, mapKey> &convoyPattern, map<ll, vector<ll>> &linkCamera, ll m,
         ll k) {

    // 输出运行时间
    clock_t beginTime, endTime;
    beginTime = clock();
    map<ll, set<vector<ll>, setKey>> preCameraListMap;

    map<vector<CameraInfo>, ll, mapCameraInfo> objSetCandidate;
    for (ll i = 0; i < singleCameraInEps.size(); i++) {
        // 在某个eps时间区间内的聚类信息
        vector<CameraInfo> objSet = singleCameraInEps[i];
        // 当前摄像头singleCameraId的聚类集合
        ll singleCameraId = objSet[0].cameraId;

        // 是否被包含，即是否需要添加到候选集中
        bool isDominated = false;

        // 保存状态
        map<vector<ll>, list<vector<CameraInfo>>, mapKey> tmpConvoyPattern;
        set<vector<ll>, setKey> preCameraList = preCameraListMap[singleCameraId];

        //for (auto it = convoyPattern.begin(); it != convoyPattern.end(); it++) {
        for (auto it = preCameraList.begin(); it != preCameraList.end(); it++) {
            vector<ll> cameraList = *it;
            bool flag = false;
            // 候选车辆集合
            list<vector<CameraInfo>> objSetSet = convoyPattern[cameraList];
            cameraList.push_back(singleCameraId);
            // 新的去过重之后的集合
            list<vector<CameraInfo>> newObjSetSet;
            for (vector<CameraInfo> tmpObjSet: objSetSet) {
                candidate_num ++;
                // 交集的个数
                vector<CameraInfo> interObjSet;
                ll objAns = 0, tmpObjAns = 0;

                while (objAns < objSet.size() && tmpObjAns < tmpObjSet.size()) {
                    CameraInfo obj = objSet[objAns];
                    CameraInfo tmpObj = tmpObjSet[tmpObjAns];
                    // 判断是否可以进行连接
                    if (obj.objId == tmpObj.objId && obj.offset == tmpObj.offset + 1) {
                        interObjSet.push_back(obj);
                        objAns++;
                        tmpObjAns++;
                    } else if (obj.objId == tmpObj.objId) {
                        if (obj.offset < tmpObj.offset + 1) {
                            objAns++;
                        } else {
                            tmpObjAns++;
                        }
                    } else if (obj.objId < tmpObj.objId) {
                        objAns++;
                    } else {
                        tmpObjAns++;
                    }
                }

                set<ll> cameraSet;
                for(CameraInfo cameraInfo:interObjSet) {
                    cameraSet.insert(cameraInfo.objId);
                }

                if (cameraSet.size() >= m) {
                    if (objSetCandidate[interObjSet] < cameraList.size()) {
                        flag = true;
                        objSetCandidate[interObjSet] = cameraList.size();

                        tmpConvoyPattern[cameraList].push_back(interObjSet);
                    }
                    //tmpConvoyPattern[cameraList].push_back(interObjSet);
                }
                if (interObjSet.size() < tmpObjSet.size()) {
                    newObjSetSet.push_back(tmpObjSet);
                }
                if (interObjSet.size() == objSet.size()) {
                    isDominated = true;
                }
            }
            if(flag) {
                for(ll ID : linkCamera[singleCameraId]) {
                    preCameraListMap[ID].insert(cameraList);
                }
            }
            cameraList.pop_back();
            convoyPattern[cameraList] = newObjSetSet;
        }

        for (auto it = tmpConvoyPattern.begin(); it != tmpConvoyPattern.end(); it++) {
            convoyPattern[it->first].splice(convoyPattern[it->first].end(), it->second);
            //convoyPattern[it->first] = it->second;
        }

        if (!isDominated) {
            vector<ll> cameraSet;
            cameraSet.push_back(singleCameraId);
//
//            list<vector<CameraInfo>> &tmpObjSetSet = convoyPattern[cameraSet];
//            auto tmpObjIt = tmpObjSetSet.begin();
//            while (tmpObjIt != tmpObjSetSet.end()) {
//                vector<CameraInfo> tmpObjSet = *tmpObjIt;
//                ll equalNum = 0, objAns = 0, tmpAns = 0;
//                while (objAns < objSet.size() && tmpAns < tmpObjSet.size()) {
//                    if (objSet[objAns].objId == tmpObjSet[tmpAns].objId &&
//                        objSet[objAns].offset == tmpObjSet[tmpAns].offset) {
//                        equalNum++;
//                        objAns++;
//                        tmpAns++;
//                    } else if (objSet[objAns].objId == tmpObjSet[tmpAns].objId &&
//                               objSet[objAns].offset < tmpObjSet[tmpAns].offset) {
//                        objAns++;
//                    } else if (objSet[objAns].objId == tmpObjSet[tmpAns].objId &&
//                               objSet[objAns].offset > tmpObjSet[tmpAns].offset) {
//                        tmpAns++;
//                    } else if (objSet[objAns].objId < tmpObjSet[tmpAns].objId) {
//                        objAns++;
//                    } else {
//                        tmpAns++;
//                    }
//                }
//                if (equalNum == objSet.size()) {
//                    isDominated = true;
//                    break;
//                } else if (equalNum == tmpObjSet.size()) {
//                    tmpObjSetSet.erase(tmpObjIt++);
//                } else {
//                    tmpObjIt++;
//                }
//            }

            if (!isDominated) {
                convoyPattern[cameraSet].push_back(objSet);
                for(ll ID : linkCamera[singleCameraId]) {
                    preCameraListMap[ID].insert(cameraSet);
                }
            }
        }
        // 去重
        // dupConvoyInSameCameraList(convoyPattern);
    }

    auto it = convoyPattern.begin();
    while (it != convoyPattern.end()) {
        if (it->first.size() < k) {
            convoyPattern.erase(it++);
        } else {
            it++;
        }
    }

    endTime = clock();
    printf("cmc time : %lf(s)\n", (double) (endTime - beginTime) / CLOCKS_PER_SEC);
}


void dupConvoyInSameCameraList(map<vector<ll>, list<vector<CameraInfo>>, mapKey> &convoyPattern) {

    // 路径的索引
    vector<vector<ll>> cameraListIndex;

    for (auto it = convoyPattern.begin(); it != convoyPattern.end(); it++) {
        // 摄像头加入索引
        cameraListIndex.push_back(it->first);
        list<vector<CameraInfo>> objSetSet = it->second;
        // 筛选后的新的车辆集合
        list<vector<CameraInfo>> newObjSetSet;

        for (vector<CameraInfo> objSet: objSetSet) {
            bool isDominated = false;
            auto newListIt = newObjSetSet.begin();
            while (newListIt != newObjSetSet.end()) {
                vector<CameraInfo> tmpObjSet = *newListIt;
                ll equalNum = 0, objAns = 0, tmpAns = 0;
                while (objAns < objSet.size() && tmpAns < tmpObjSet.size()) {
                    if (objSet[objAns].objId == tmpObjSet[tmpAns].objId &&
                        objSet[objAns].offset == tmpObjSet[tmpAns].offset) {
                        equalNum++;
                        objAns++;
                        tmpAns++;
                    } else if (objSet[objAns].objId == tmpObjSet[tmpAns].objId &&
                               objSet[objAns].offset < tmpObjSet[tmpAns].offset) {
                        objAns++;
                    } else if (objSet[objAns].objId == tmpObjSet[tmpAns].objId &&
                               objSet[objAns].offset > tmpObjSet[tmpAns].offset) {
                        tmpAns++;
                    } else if (objSet[objAns].objId < tmpObjSet[tmpAns].objId) {
                        objAns++;
                    } else {
                        tmpAns++;
                    }
                }
                if (equalNum == objSet.size()) {
                    isDominated = true;
                    break;
                } else if (equalNum == tmpObjSet.size()) {
                    newObjSetSet.erase(newListIt++);
                } else {
                    newListIt++;
                }
            }
            if (!isDominated) {
                newObjSetSet.push_back(objSet);
            }
        }
        it->second = newObjSetSet;
    }

    for (ll i = 0; i < cameraListIndex.size(); i++) {
        for (ll j = 0; j < cameraListIndex.size(); j++) {
            if (cameraListIndex[i].size() >= cameraListIndex[j].size()) {
                continue;
            }
            vector<ll> cameraList = cameraListIndex[i];
            vector<ll> faCameraList = cameraListIndex[j];
            ll pos, countNum = 0;
            for (ll ans = 0; ans < faCameraList.size(); ans++) {
                if (faCameraList[ans] == cameraList[countNum]) {
                    countNum++;
                } else {
                    countNum = 0;
                }
                if (countNum == cameraList.size()) {
                    pos = faCameraList.size() - ans - 1;
                    break;
                }
            }

            if (countNum == cameraList.size()) {
                list<vector<CameraInfo>> &objSetSet = convoyPattern[cameraList];
                list<vector<CameraInfo>> tmpObjSetSet = convoyPattern[faCameraList];

                auto objIt = objSetSet.begin();
                while (objIt != objSetSet.end()) {
                    bool isDominated = false;
                    vector<CameraInfo> objSet = *objIt;
                    for (auto tmpObjSet: tmpObjSetSet) {
                        ll equalNum = 0, objAns = 0, tmpAns = 0;
                        // 进行去重时的比较
                        while (objAns < objSet.size() && tmpAns < tmpObjSet.size()) {
                            if (objSet[objAns].objId == tmpObjSet[tmpAns].objId &&
                                objSet[objAns].offset == tmpObjSet[tmpAns].offset - pos) {
                                equalNum++;
                                objAns++;
                                tmpAns++;
                            } else if (objSet[objAns].objId == tmpObjSet[tmpAns].objId &&
                                       objSet[objAns].offset < tmpObjSet[tmpAns].offset - pos) {
                                objAns++;
                            } else if (objSet[objAns].objId == tmpObjSet[tmpAns].objId &&
                                       objSet[objAns].offset > tmpObjSet[tmpAns].offset - pos) {
                                tmpAns++;
                            } else if (objSet[objAns].objId < tmpObjSet[tmpAns].objId) {
                                objAns++;
                            } else {
                                tmpAns++;
                            }
                        }
                        if (equalNum == objSet.size()) {
                            isDominated = true;
                            break;
                        }
                    }
                    if (isDominated) {
                        objSetSet.erase(objIt++);
                    } else {
                        objIt++;
                    }
                }
            }
        }
    }
}

void dupConvoyInFaCamera(map<vector<ll>, list<vector<CameraInfo>>, mapKey> &convoyPattern) {

    // 路径的索引
    vector<vector<ll>> cameraListIndex;
    for (auto it = convoyPattern.begin(); it != convoyPattern.end(); it++) {
        // 摄像头加入索引
        cameraListIndex.push_back(it->first);
    }

    for (ll i = 0; i < cameraListIndex.size(); i++) {
        for (ll j = 0; j < cameraListIndex.size(); j++) {
            if (cameraListIndex[i].size() >= cameraListIndex[j].size()) {
                continue;
            }
            vector<ll> cameraList = cameraListIndex[i];
            vector<ll> faCameraList = cameraListIndex[j];
            ll pos, countNum = 0;
            for (ll ans = 0; ans < faCameraList.size(); ans++) {
                if (faCameraList[ans] == cameraList[countNum]) {
                    countNum++;
                } else {
                    countNum = 0;
                }
                if (countNum == cameraList.size()) {
                    pos = faCameraList.size() - ans - 1;
                    break;
                }
            }

            if (countNum == cameraList.size()) {
                list<vector<CameraInfo>> &objSetSet = convoyPattern[cameraList];
                list<vector<CameraInfo>> tmpObjSetSet = convoyPattern[faCameraList];

                auto objIt = objSetSet.begin();
                while (objIt != objSetSet.end()) {
                    bool isDominated = false;
                    vector<CameraInfo> objSet = *objIt;
                    for (auto tmpObjSet: tmpObjSetSet) {
                        ll equalNum = 0, objAns = 0, tmpAns = 0;
                        while (objAns < objSet.size() && tmpAns < tmpObjSet.size()) {
                            if (objSet[objAns].objId == tmpObjSet[tmpAns].objId &&
                                objSet[objAns].offset == tmpObjSet[tmpAns].offset - pos) {
                                equalNum++;
                                objAns++;
                                tmpAns++;
                            } else if (objSet[objAns].objId == tmpObjSet[tmpAns].objId &&
                                       objSet[objAns].offset < tmpObjSet[tmpAns].offset - pos) {
                                objAns++;
                            } else if (objSet[objAns].objId == tmpObjSet[tmpAns].objId &&
                                       objSet[objAns].offset > tmpObjSet[tmpAns].offset - pos) {
                                tmpAns++;
                            } else if (objSet[objAns].objId < tmpObjSet[tmpAns].objId) {
                                objAns++;
                            } else {
                                tmpAns++;
                            }
                        }
                        if (equalNum == objSet.size()) {
                            isDominated = true;
                            break;
                        }
                    }
                    if (isDominated) {
                        objSetSet.erase(objIt++);
                    } else {
                        objIt++;
                    }
                }
            }

        }
    }
}

void split(vector<CameraInfo> objSet, vector<CameraInfo> newObjSet, vector<ll>cameraList,list<vector<CameraInfo>> &objSetList) {
    if(newObjSet.size() == cameraList.size()) {
        objSetList.push_back(newObjSet);
        return;
    }
    for(CameraInfo obj:objSet) {
        if(obj.objId == cameraList[newObjSet.size()]) {
            newObjSet.push_back(obj);
            split(objSet, newObjSet,cameraList,objSetList);
            newObjSet.pop_back();
        }
    }
}

void dfs(map<vector<ll>, list<vector<CameraInfo>>, mapKey> &convoyPattern) {
    for(auto it = convoyPattern.begin(); it != convoyPattern.end(); it++) {
        list<vector<CameraInfo>> objSetList = it->second;
        list<vector<CameraInfo>> newObjSetList;
        for(vector<CameraInfo> objSet : objSetList) {
            vector<CameraInfo> newObjSet;
            vector<ll> cameraList;
            for(CameraInfo obj:objSet) {
                if(cameraList.size() && obj.objId != cameraList[cameraList.size() - 1]) {
                    cameraList.push_back(obj.objId);
                }
            }
            split(objSet, newObjSet, cameraList, newObjSetList);
        }
        it->second = newObjSetList;
    }
}
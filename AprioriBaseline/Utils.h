//ll
// Created by lenovo on 2022/12/19.
//

#ifndef CONVOYBASELINE_UTIL_H
#define CONVOYBASELINE_UTIL_H

#endif //CONVOYBASELINE_UTIL_H

#include "HeadFiles.h"
#include "customizMap.h"

ll candidate_num;
ll compare_num;
string path;
vector<string> fileName;
double dupTime;
vector<vector<vector<ll>>> objectRoute;

/**
 * 根据开始时间自定义排序
 * @param p1
 * @param p2
 * @return
 */
bool cmpRoutePair(pair<ll, ll> &p1, pair<ll, ll> &p2) {
    return objectRoute[p1.first][p1.second][1] < objectRoute[p2.first][p2.second][1];
}

map<ll, ll> objectId;

bool cmpCanRoute(pair<ll, ll> &p1, pair<ll, ll> &p2) {
    if (objectId[p1.first] != objectId[p2.first]) {
        return objectId[p1.first] < objectId[p2.first];
    } else {
        return p1.second < p2.second;
    }
}

map<ll, vector<pair<ll, ll>>> comCamera;
map<vector<ll>, vector<vector<pair<ll, ll>>>, cmp_key> comTimeRoute;

map<ll, set<ll>> nextCamera;
//用来保存最后得到的convoy结果集
map<vector<ll>, vector<vector<pair<ll, ll>>>, cmp_key> convoyPattern;
ll m, k, eps;


//获取文本对应的object_id
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
 * 读取path路径下的文件并用filenames保存
 * @param path
 * @param filenames
 */
void GetFileNames(string path, vector<string> &filenames) {
    DIR *pDir;
    struct dirent *ptr;
    if (!(pDir = opendir(path.c_str())))
        return;
    while ((ptr = readdir(pDir)) != 0) {
        if (strcmp(ptr->d_name, ".") != 0 && strcmp(ptr->d_name, "..") != 0)
            filenames.push_back(path + "/" + ptr->d_name);
    }
    closedir(pDir);
}

/**
 * 判断候选集中个数是否大于等于m
 * @param route
 * @param m
 * @return
 */
bool checkNum(vector<pair<ll, ll>> route, ll m) {
    set<ll> s;
    for (ll i = 0; i < route.size(); i++) {
        s.insert(route[i].first);
    }
    if (s.size() >= m) {
        return true;
    } else {
        return false;
    }
}

void newPrintComTimeRoute(map<vector<ll>, vector<vector<pair<ll, ll>>>, cmp_key> &comTimeRoute,
                          map<ll, ll> &objectId) {
    ll ans = 0;
    ll ans1 = 0;
    vector<vector<pair<ll, ll>>> objectVec;
    vector<ll> cameraVec;
    for (auto it = comTimeRoute.begin(); it != comTimeRoute.end(); it++) {
        ans1++;
        cameraVec = it->first;
/*        printf("continues cameraId : \n");
        for (ll i = 0; i < cameraVec.size(); i++) {
            printf("%lld ", cameraVec[i]);
        }
        printf("\n");
        printf("object set : \n");*/
        objectVec = it->second;
        for (vector<pair<ll, ll>> object: objectVec) {
            ans++;
/*            for (pair<ll, ll> objCamera: object) {
                //printf("%lld %lld \n", objCamera.first, objCamera.second);
                printf("%lld %lld \n", objectId[objCamera.first], objCamera.second);
            }
            printf("--------------------\n");*/
        }


/*        printf("======================\n");*/
    }
    printf("total convoy number is : %lld\n", ans);
    printf("total continues route number is : %lld\n", ans1);
}

/**
 * objectRoute存储每一辆车的路径信息
 * objectRoute--->route--->cameraInfo
 * 每一条route代表一辆车的信息
 * 每一个cameraInfo信息 : cameraId,startTime,endTime。
 * @param objectRoute
 * @param files
 * @param objectId
 */
void getData(vector<vector<vector<ll>>> &objectRoute, vector<string> &files, map<ll, ll> &objectId) {
    // objectRoute表示所有车辆的路径信息

    // route表示单个车辆的路径信息
    vector<vector<ll>> route;
    // cameraInfo 表示单个摄像头id，进入时间startTime，离开时间endTime
    vector<ll> cameraInfo;

    string file;
    ll num = 0;
    for (ll i = 0; i < files.size(); i++) {
        route.clear();
        // objectId为对应车辆的ID
        file = files[i];
        ll id = getId(file);
        objectId[i] = id;
        // 文件流输入
        ifstream fin;
        string buffer;
        fin.open(file, ios::in);
        while (getline(fin, buffer)) {
            cameraInfo.clear();
            num = 0;
            for (char c: buffer) {
                if (c == ',' || c == '\r') {
                    cameraInfo.push_back(num);
                    num = 0;
                } else {
                    num = 10 * num + c - '0';
                }
            }
            route.push_back(cameraInfo);
        }
        objectRoute.push_back(route);
    }
    return;
}


/**
 * @param comCamera ：经过摄像头下车辆的信息，用pair保存,first为车辆编号,second为route中cameraInfo的编号
 * @param nextCamera : 存储摄像头的下一个可能出现过的摄像头编号ID
 * @param objectRoute
 */
void getCommonRoute(map<ll, vector<pair<ll, ll>>> &comCamera, map<ll, set<ll>> &nextCamera,
                    vector<vector<vector<ll>>> &objectRoute) {
    for (ll id = 0; id < objectRoute.size(); id++) {
        for (ll i = 0; i < objectRoute[id].size(); i++) {
            //将连续的两个cameraId保存记录
            if (i != objectRoute[id].size() - 1) {
                nextCamera[objectRoute[id][i][0]].insert(objectRoute[id][i + 1][0]);
            }
            comCamera[objectRoute[id][i][0]].push_back(make_pair(id, i));
        }
    }
}


/**
 * 单个摄像头下的车辆时间聚类
 * @param comTimeRoute
 * @param m
 * @param exps
 */
void getComTimeIntervalRoute(map<vector<ll>, vector<vector<pair<ll, ll>>>, cmp_key> &comTimeRoute, ll m, ll eps) {

    vector<ll> conCamera;
    vector<pair<ll, ll>> route;
    vector<pair<ll, ll>> parRoute;
    vector<pair<ll, ll>> canRoute;
    ll left = 0, right = 0;
    bool flag = 0;
    //滑动窗口将所有单个cameraId下的车辆根据startTime划分
    for (auto it = comCamera.begin(); it != comCamera.end(); it++) {
        conCamera.clear();
        conCamera.push_back(it->first);
        route = it->second;
        left = right = 0;
        parRoute.clear();
        sort(route.begin(), route.end(), cmpRoutePair);
        for (left = 0; left < route.size(); left++) {
            if (left) {
                parRoute.erase(parRoute.begin());
            }
            while (right < route.size() && objectRoute[route[right].first][route[right].second][1] -
                                           objectRoute[route[left].first][route[left].second][1] <= eps) {
                parRoute.push_back(route[right++]);
                flag = 1;
            }
            if (flag) {
                flag = 0;
                if (checkNum(parRoute, m)) {
                    // 添加满足条件的
                    canRoute = parRoute;
                    // 对车辆集合进行排序便于进一步操作
                    sort(canRoute.begin(), canRoute.end(), cmpCanRoute);
                    comTimeRoute[conCamera].push_back(canRoute);
                }
            }
            if (right == route.size()) {
                break;
            }
        }
    }
}


/**
 * k-len的objSet与nextObjSet合并为k+1-len的新的摄像头路径集合
 * @param objSet
 * @param nextObjSet
 * @param m
 * @return
 */
vector<vector<pair<ll, ll>>> connectRoute(vector<vector<pair<ll, ll>>> objSet,
                                          vector<vector<pair<ll, ll>>> nextObjSet,
                                          ll m) {
    // canObjSet : 聚类求交集之后保存的聚类
    vector<vector<pair<ll, ll>>> canObjSet;
    vector<vector<pair<ll, ll>>> opObjSet;
    vector<pair<ll, ll>> canRoute;
    vector<pair<ll, ll>> opRoute;

    for (vector<pair<ll, ll>> route: objSet) {
        // 循环遍历求交集
        for (vector<pair<ll, ll>> nextRoute: nextObjSet) {


            ll i = 0, j = 0;
            canRoute.clear();
            while (i < route.size() && j < nextRoute.size()) {
                if (objectId[route[i].first] == objectId[nextRoute[j].first]) {
                    if (route[i].second + 1 == nextRoute[j].second) {
                        canRoute.push_back(route[i]);
                        i++;
                        j++;
                    } else if (route[i].second + 1 < nextRoute[j].second) {
                        i++;
                    } else {
                        j++;
                    }
                } else if (objectId[route[i].first] < objectId[nextRoute[j].first]) {
                    i++;
                } else {
                    j++;
                }
            }

            if (checkNum(canRoute, m)) {
                // 去重并选最大集
                clock_t start_time, end_time;
                start_time = clock();
                opObjSet = canObjSet;
                auto it = opObjSet.begin();
                bool flag = 1;
                while (it != opObjSet.end()) {
                    opRoute = *it;
                    i = j = 0;
                    // canCount与opCount分别计算两个集合中相同的个体数量
                    ll canCount = 0, opCount = 0;
                    while (i < opRoute.size() && j < canRoute.size()) {
                        if (objectId[opRoute[i].first] == objectId[canRoute[j].first]) {
                            if (opRoute[i].second == canRoute[j].second) {
                                i++;
                                j++;
                                canCount++;
                                opCount++;
                            } else if (opRoute[i].second < canRoute[j].second) {
                                i++;
                            } else {
                                j++;
                            }
                        } else if (objectId[opRoute[i].first] < objectId[canRoute[j].first]) {
                            i++;
                        } else {
                            j++;
                        }
                    }
                    if (canCount == canRoute.size()) {
                        flag = 0;
                        it++;
                    } else if (opCount == opRoute.size()) {
                        it = opObjSet.erase(it);
                    } else {
                        it++;
                    }
                }
                if (flag) {
                    opObjSet.push_back(canRoute);
                }
                canObjSet = opObjSet;
                end_time = clock();
                dupTime += (double) (end_time - start_time) / CLOCKS_PER_SEC;
            }
        }
    }
    return canObjSet;
}

void dupInnerConvoy(vector<vector<pair<ll, ll>>> &newObjSet) {
    vector<vector<pair<ll, ll>>> opObjSet;
    for (ll i = 0; i < newObjSet.size(); i++) {
        bool flag = 1;
        vector<pair<ll, ll>> newObj = newObjSet[i];
        auto it = opObjSet.begin();
        while (it != opObjSet.end()) {
            vector<pair<ll, ll>> opObj = *it;
            ll newCount = 0, opCount = 0;
            ll i = 0, j = 0;
            while (i < newObj.size() && j < opObj.size()) {
                if (newObj[i].first == opObj[j].first) {
                    if (newObj[i].second == opObj[j].second) {
                        newCount++;
                        opCount++;
                        i++;
                        j++;
                    } else if (newObj[i].second < opObj[j].second) {
                        i++;
                    } else {
                        j++;
                    }
                } else if (newObj[i].first < opObj[j].first) {
                    i++;
                } else {
                    j++;
                }
            }
            if (newCount == newObj.size()) {
                flag = 0;
                it++;
            } else if (opCount == opObj.size()) {
                it = opObjSet.erase(it);
            } else {
                it++;
            }
        }
        if (flag) {
            opObjSet.push_back(newObj);
        }
    }
    newObjSet = opObjSet;
}

// convoyPattern 去重 dupRemoveConvoy
void dupRemoveConvoy(vector<ll> &conCamera, vector<vector<pair<ll, ll>>> &newObjSet) {


    // 如果插入的是 k - len 长度的convoy，只需要在 k - 1-len的convoy差去重
    // 也即去掉头尾的两个convoy
    vector<vector<ll>> cameraVec;
    vector<ll> subCamera = conCamera;
    subCamera.pop_back();
    cameraVec.push_back(subCamera);
    subCamera = conCamera;
    subCamera.erase(subCamera.begin());
    cameraVec.push_back(subCamera);
    ll ans = 0;
    // 定义map得到的车辆集合
    vector<pair<ll, ll>> route;

    // 遍历判断map是否包含了这两个convoy
    for (vector<ll> camera: cameraVec) {
        if (convoyPattern.find(camera) != convoyPattern.end()) {
            // 目前已经被记录的长度为 k-1 的连续摄像头的convoyPattern
            vector<vector<pair<ll, ll>>> &convoySet = convoyPattern[camera];
            auto it = convoySet.begin();
            while (it != convoySet.end()) {
                route = *it;
                bool flag = 0;
                for (vector<pair<ll, ll>> objRoute: newObjSet) {
                    if (route.size() == objRoute.size()) {
                        bool subFlag = 1;
                        for (ll i = 0; i < route.size(); i++) {
                            if (!(objectId[route[i].first] == objectId[objRoute[i].first]
                                  && route[i].second - ans == objRoute[i].second)) {
                                subFlag = 0;
                                break;
                            }
                        }
                        if (subFlag == 1) {
                            flag = 1;
                        }
                    }
                }
                if (flag) {
                    it = convoySet.erase(it);
                } else {
                    it++;
                }
            }
            if (convoySet.size() == 0) {
                convoyPattern.erase(camera);
            }
        }
        ans++;
    }

}

void dfs(vector<vector<pair<ll, ll>>> &newObjectSet, vector<pair<ll, ll>> &canObject,
         map<ll, vector<pair<ll, ll>>> &oMap, ll pos) {
    if (pos == oMap.size()) {
        newObjectSet.push_back(canObject);
        return;
    }
    vector<pair<ll, ll>> sameObject = oMap[pos];
    for (ll i = 0; i < sameObject.size(); i++) {
        canObject.push_back(sameObject[i]);
        dfs(newObjectSet, canObject, oMap, pos + 1);
        canObject.pop_back();
    }
}

void splitObjSet(vector<vector<pair<ll, ll>>> &objSet) {
    vector<vector<pair<ll, ll>>> newObjectSet;
    for (vector<pair<ll, ll>> object: objSet) {
        map<ll, vector<pair<ll, ll>>> oMap;
        ll step = -1;
        for (ll i = 0; i < object.size(); i++) {
            if (!i || object[i].first != object[i - 1].first) {
                step++;
            }
            oMap[step].push_back(object[i]);
        }
        vector<pair<ll, ll>> canObject;
        dfs(newObjectSet, canObject, oMap, 0);
    }
    objSet = newObjectSet;
}

/**
 * 扩展得到所有满足条件的convoy
 * @param k
 * @param m
 */
void getConvoyPattern(ll k, ll m) {
    bool flag = 1;
    ll curLen = 1;
    ll cameraId;
    vector<ll> conCamera;
    vector<ll> linkCamera;
    vector<vector<pair<ll, ll>>> objSet;
    vector<vector<pair<ll, ll>>> nextObjSet;
    vector<vector<pair<ll, ll>>> newObjSet;

    // 分裂 [1_1,1_2,2,3,4] 分成 [1_1,2,3,4], [1_2,2,3,4]
/*
    for (auto it = comTimeRoute.begin(); it != comTimeRoute.end(); it++) {
        objSet = it->second;
        splitObjSet(objSet);
        it->second = objSet;
    }
*/

    // 扩展主算法
    while (flag) {
        // cout << "camera len:" << curLen << endl;
        // newPrintComTimeRoute(comTimeRoute, objectId);
        // cout << "=====================" << endl;
        flag = 0;
        map<vector<ll>, vector<vector<pair<ll, ll>>>, cmp_key> newTimeRoute;


    
        curLen++;
        ll next_camera_number = 0;
        for (auto it = comTimeRoute.begin(); it != comTimeRoute.end(); it++) {
            // conCamera : 摄像头序列
            conCamera = it->first;
            // objSet : 车辆聚类集合
            objSet = it->second;

            // 求candidate 候选集数目
            // candidate_num += objSet.size();

            // linkCamera : 摄像头下一个连接的摄像头序列，例如:abc + bcd = abcd
            linkCamera = conCamera;
            linkCamera.erase(linkCamera.begin());
            cameraId = conCamera[conCamera.size() - 1];
            // k - len 的 convoy 合并为 k + 1 - len 的 convoy
            for (ll newCameraId: nextCamera[cameraId]) {
                linkCamera.push_back(newCameraId);
                if (comTimeRoute.find(linkCamera) != comTimeRoute.end()) {

                    nextObjSet = comTimeRoute[linkCamera];
                    // newObjSet = nextObjSet 与 objSet 集合的交集
                    newObjSet = connectRoute(objSet, nextObjSet, m);
                    // 求取交集次数
                    candidate_num += objSet.size()*nextObjSet.size();
                    if (newObjSet.size() != 0) {
                        flag = 1;
                        conCamera.push_back(newCameraId);
                        // splitObjSet(newObjSet);

                        clock_t start_time, end_time;
                        start_time = clock();
                        dupInnerConvoy(newObjSet);
                        end_time = clock();
                        dupTime += (double) (end_time - start_time) / CLOCKS_PER_SEC;

                        // 新建索引
                        newTimeRoute[conCamera] = newObjSet;
                        if (curLen >= k) {
                            // 加入convoyPattern，但是还需要考虑去重
                            // 1. 首先分裂 [1_1,1_2,2,3,4] 分成 [1_1,2,3,4], [1_2,2,3,4]

                            // 2. 去重
                            clock_t start_time, end_time;
                            start_time = clock();
                            dupRemoveConvoy(conCamera, newObjSet);
                            end_time = clock();
                            dupTime += (double) (end_time - start_time) / CLOCKS_PER_SEC;
                            // 3. 加入结果集
                            convoyPattern[conCamera] = newObjSet;
                        }
                        conCamera.pop_back();
                    }
                }
                linkCamera.pop_back();
            }
        }
        comTimeRoute = newTimeRoute;
    }
}

/**
 * 最后一步去重操作，主要是为了解决摄像头序列间隔有相同的情况
 * @param convoyPattern
 */
void dupConvoy(map<vector<ll>, vector<vector<pair<ll, ll>>>, cmp_key> &convoyPattern) {
    vector<vector<ll>> cameraSeqSet;
    vector<ll> cameraSet;
    // invertMap : 倒排表
    map<ll, set<ll>> invertMap;
    for (auto it = convoyPattern.begin(); it != convoyPattern.end(); it++) {
        cameraSeqSet.push_back(it->first);
    }
    for (ll i = 0; i < cameraSeqSet.size(); i++) {
        cameraSet = cameraSeqSet[i];
        for (ll id: cameraSet) {
            // 将摄像头编号id所在的序列编号记录
            invertMap[id].insert(i);
        }
    }
    for (ll ans = 0; ans < cameraSeqSet.size(); ans++) {
        cameraSet = cameraSeqSet[ans];
        set<ll> interSet = invertMap[cameraSet[0]];
        set<ll> opSet;
        for (ll id: cameraSet) {
            opSet.clear();
            set_intersection(interSet.begin(), interSet.end(), invertMap[id].begin(), invertMap[id].end(),
                             inserter(opSet, opSet.begin()));
            interSet = opSet;
        }
        for (ll pos: interSet) {
            if (pos != ans) {
                vector<vector<pair<ll, ll>>> objSet = convoyPattern[cameraSet];
                vector<vector<pair<ll, ll>>> faObjSet = convoyPattern[cameraSeqSet[pos]];
                vector<vector<pair<ll, ll>>> newObjSet;
                newObjSet.clear();
                for (vector<pair<ll, ll>> obj: objSet) {
                    bool flag = 1;
                    for (vector<pair<ll, ll>> faObj: faObjSet) {
                        ll i = 0, j = 0;
                        while (i < obj.size() && j < faObj.size()) {
                            if (obj[i].first == faObj[j].first
                                && obj[i].second >= faObj[j].second
                                && obj[i].second + cameraSet.size() <= faObj[j].second + cameraSeqSet[pos].size()) {
                                i++;
                            }
                            j++;
                        }
                        if (i == obj.size()) {
                            flag = 0;
                            break;
                        }
                    }
                    if (flag) {
                        newObjSet.push_back(obj);
                    }
                }
                convoyPattern[cameraSet] = newObjSet;
            }
        }
        if (!convoyPattern[cameraSet].size()) {
            convoyPattern.erase(cameraSet);
        }
    }

}


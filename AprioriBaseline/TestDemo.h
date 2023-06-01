//
// Created by lenovo on 2022/12/19.
//

#ifndef CONVOYBASELINE_TESTDEMO_H
#define CONVOYBASELINE_TESTDEMO_H

#endif //CONVOYBASELINE_TESTDEMO_H

#include "HeadFiles.h"

void printObjectRoute(vector<vector<vector<ll>>> objectRoute, map<ll, ll> objectId) {
    for (ll i = 0; i < objectRoute.size(); i++) {
        printf("ObjectId = %lld\n", objectId[i]);
        printf("===============================\n");
        for (ll j = 0; j < objectRoute[i].size(); j++) {
            for (ll k = 0; k < objectRoute[i][j].size(); k++) {
                if (k) {
                    printf(" ");
                }
                printf("%lld", objectRoute[i][j][k]);
            }
            printf("\n");
        }
    }
}

//输出得到的文件夹信息
void printFileName(vector<string> files) {
    for(ll i = 0; i < files.size(); i++) {
        cout<<files[i]<<endl;
    }
}

void printCommonRoute(map<ll, vector<pair<ll, ll>>> &comCamera,
                      map<ll, set<ll>> &nextCamera, map<ll, ll> &objectId) {

    vector<pair<ll, ll>> object;
    set<ll> cameraVec;
    for (auto it = comCamera.begin(); it != comCamera.end(); it++) {
        printf("objects pass around the camera, cameraId : %lld\n", it->first);

        printf("object set : \n");
        object = it->second;
        for (pair<ll, ll> objCamera: object) {
            printf("%lld %lld \n", objectId[objCamera.first], objCamera.second);
        }
        printf("======================\n");
    }

    printf("next camera sets:\n");
    for (auto it = nextCamera.begin(); it != nextCamera.end(); it++) {
        printf("cameraId : %lld\n", it->first);
        printf("nextCamera:\n");
        cameraVec = it->second;
        for (ll id: cameraVec) {
            printf("%lld ", id);
        }
        printf("\n");
    }
}

void printComTimeRoute(map<vector<ll>, vector<vector<pair<ll, ll>>>, cmp_key> &comTimeRoute,
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

void printVectorPair(vector<vector<pair<ll, ll>>> set) {
    for (vector<pair<ll, ll>> route: set) {
        for (ll i = 0; i < route.size(); i++) {
            cout << route[i].first << "," << route[i].second << " ";
        }
        cout << endl;
    }
}

void testPrintVectorPair() {
    vector<pair<ll, ll>> p1;
    vector<pair<ll, ll>> p2;
    vector<vector<pair<ll, ll>>> set1;
    vector<vector<pair<ll, ll>>> set2;
/*    for (ll i = 1; i <= 7; i++) {
        p1.push_back(make_pair(i, i));
    }
    set1.push_back(p1);
    p1.clear();
    for (ll i = 1; i <= 7; i++) {
        p1.push_back(make_pair(i, i+3));
    }
    set1.push_back(p1);
    for (ll i = 2; i <= 8; i++) {
        p2.push_back(make_pair(i, i + 1));
    }
    set2.push_back(p2);
    p2.clear();
    for (ll i = 2; i <= 8; i++) {
        p2.push_back(make_pair(i, i+4));
    }
    set2.push_back(p2);
    set2 = connectRoute(set1, set2, 3);*/
    set1.push_back(p1);
    set2.push_back(p2);
    set2 = connectRoute(set1, set2, m);
    cout << set2.size() << endl;
    printVectorPair(set2);
}

void printNextCameraId() {
    for (auto it = nextCamera.begin(); it != nextCamera.end(); it++) {
        cout << "cameraId : " << it->first<< endl;
        cout << "next camera is :" << endl;
        set<ll> cameraId = it->second;
        for (ll camera: cameraId) {
            cout<<camera<<" ";
        }
        cout<<endl;
    }
}
//
// Created by lenovo on 2023/4/11.
//

#ifndef CMC_TESTDEMO_H
#define CMC_TESTDEMO_H

#endif //CMC_TESTDEMO_H

#include "utils.h"

/**
 * 测试获得的文件路径
 * @param fileNames
 */
void testFileNames(vector<string> fileNames) {
    cout << "file size: " << fileNames.size() << endl;
    for (int i = 0; i < fileNames.size(); i++) {
        cout << fileNames[i] << endl;
    }
}

void testObjPath(vector<CameraInfo> &objPath) {
    for (ll i = 0; i < objPath.size(); i++) {
        CameraInfo cameraInfo = objPath[i];
        cout << cameraInfo.objId << ", " << cameraInfo.offset << ", " << cameraInfo.cameraId << ", "
             << cameraInfo.startTime << endl;
    }
}

void printSingleCamera(map<ll, vector<CameraInfo>> &singleCamera) {
    for (auto it = singleCamera.begin(); it != singleCamera.end(); it++) {
        cout << "camera id = " << it->first << endl;
        cout << "------------" << endl;
        vector<CameraInfo> objSet = it->second;
        for (CameraInfo cameraInfo: objSet) {
            cout << cameraInfo.objId << ", " << cameraInfo.cameraId << endl;
        }
        cout << "============" << endl;
    }
}


void testCluster(vector<vector<CameraInfo>> &objCluster) {
    cout << "objCluster size: " << objCluster.size() << endl;
//    for(ll i = 0; i < objCluster.size(); i++) {
//        vector<CameraInfo> objPath = objCluster[i];
//        testObjPath(objPath);
//        cout<<"-----------------------"<<endl;
//    }
}

void printConvoy(map<vector<ll>, list<vector<CameraInfo>>, mapKey> &convoyPattern) {
    ll num = 0;
    for (auto it = convoyPattern.begin(); it != convoyPattern.end(); it++) {
        if (!it->second.size()) {
            continue;
        }
        num += it->second.size();
//        vector<ll> cameraSet = it->first;
//        for (ll cameraId: cameraSet) {
//            printf("%lld ", cameraId);
//        }
//        printf("\n");
//        printf("objSet:\n");
//        list<vector<CameraInfo>> objSetSet = it->second;
//        for (vector<CameraInfo> objSet: objSetSet) {
//            cout << "objSet size:" << objSet.size() << endl;
//            for (CameraInfo obj: objSet) {
//                printf("%lld,%lld  ", obj.objId, obj.offset);
//            }
//            printf("\n");
//        }
//        printf("=====================================\n");
    }
    printf("total convoy numbers: %lld\n", num);
}

void printSingleCameraInEps(vector<vector<CameraInfo>> &singleCameraInEps) {
    for (vector<CameraInfo> singleCamera: singleCameraInEps) {
        bool flag = false;
        for (CameraInfo cameraInfo: singleCamera) {
            if (cameraInfo.objId == 19 || cameraInfo.objId == 20 || cameraInfo.objId == 31) {
                flag = true;
            }
        }
        if (flag) {
            cout << "cameraID: " << singleCamera[0].cameraId << endl;
            for (CameraInfo cameraInfo: singleCamera) {
                cout << cameraInfo.objId << "," << cameraInfo.startTime << " ";
            }
            cout << endl;
            cout << "-----------------" << endl;
        }
    }
}
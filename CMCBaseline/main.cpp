#include "testdemo.h"

// m:聚类车辆个数, k:连续摄像头长度, eps:时间阈值
ll m, k, eps;
string path;
vector<string> fileNames;
vector<CameraInfo> objPath; //车辆路径


// 单个摄像头下的路径信息
map<ll, vector<CameraInfo>> singleCamera;

// 单个摄像头下根据eps的聚类信息
vector<vector<CameraInfo>> singleCameraInEps;
map<vector<ll>, list<vector<CameraInfo>>, mapKey> convoyPattern; //保存convoy模式
map<ll, vector<ll>> linkCamera;

int main(int argc, char *argv[]) {
    clock_t beginTime, endTime;
    beginTime = clock();
    sscanf(argv[1], "%lld", &m);
    sscanf(argv[2], "%lld", &k);
    sscanf(argv[3], "%lld", &eps);
    string index = argv[4];

    printf("m = %lld, k = %lld, eps = %lld\n", m, k, eps);
    path = "../../../dataset/";
    path += index;
    // path += "chengdu_1020";
    // path += "singapore";
    // path += "carla/t16_c600_len14";
    // path += "video_sg/t14_c600_len16";
    cout << "path:" << path << endl;

    // 获得文件路径
    getFileNames(path, fileNames);
    // testFileNames(fileNames);

    // 获取路径信息
    getDataByFile(singleCamera, objPath, fileNames, linkCamera);
    //printSingleCamera(singleCamera);

    getClusterByEps(singleCamera, m, eps, singleCameraInEps);
    //printSingleCameraInEps(singleCameraInEps);

    // 改进cmc算法
    cmc(singleCameraInEps, convoyPattern, linkCamera, m, k);

    //dfs(convoyPattern);

    dupConvoyInSameCameraList(convoyPattern);
    //dupConvoyInFaCamera(convoyPattern);
    //dupConvoy(convoyPattern);
    //dupConvoyDifferCamera(convoyPattern, m);
    printConvoy(convoyPattern);
    endTime = clock();
    printf("total time : %lf(s)\n", (double) (endTime - beginTime) / CLOCKS_PER_SEC);
    printf("candidate number: %lld\n", candidate_num);
    return 0;
}

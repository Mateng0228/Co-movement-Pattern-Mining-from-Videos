#include <iostream>
#include "Utils.h"
#include "TestDemo.h"


int main(int argc, char *argv[]) {
    clock_t start_time, end_time;
    start_time = clock();
    string index = argv[1];
    sscanf(argv[2], "%lld", &m);
    sscanf(argv[3], "%lld", &k);
    sscanf(argv[4], "%lld", &eps);

    path = "datasets/";
    path += index;
    cout<<"m = "<<m<<", k = "<<k<<", eps = "<<eps<<endl;
    cout<<"Path = "<<path<<endl;
    GetFileNames(path, fileName);
    getData(objectRoute, fileName, objectId);


    // 测试输出文件路径
    // printFileName(fileName);

    // @Test:输出每个车辆的路径信息
    // printObjectRoute(objectRoute, objectId);

    // comCamera : 每个摄像头下经过的所有的object集合，用pair表示，first是objectId， second用来表示startTime
    // nextCamera : 每个摄像头可能经过的后一个摄像头
    getCommonRoute(comCamera, nextCamera, objectRoute);

    // @Test
    // printCommonRoute(comCamera, nextCamera, objectId);

    // comTimeRoute 目前为连续1个摄像头的所有满足m与exps的convoy模式
    getComTimeIntervalRoute(comTimeRoute, m, eps);

    // @Test
    // printComTimeRoute(comTimeRoute, objectId);
    // printNextCameraId();
    // testPrintVectorPair();

    // 获得convoy模式
    getConvoyPattern(k, m);

    // 最后一步对convoy进行去重
    clock_t s_time, e_time;
    s_time = clock();
    dupConvoy(convoyPattern);
    e_time = clock();
    dupTime += (double) (e_time - s_time) / CLOCKS_PER_SEC;
    // printComTimeRoute(comTimeRoute, objectId);
    printComTimeRoute(convoyPattern, objectId);

    end_time = clock();
    printf("running time : %lf(s)\n", (double) (end_time - start_time) / CLOCKS_PER_SEC);
    printf("duplicating time : %lf(s)\n", dupTime);
    printf("candidate number: %lld\n", candidate_num);


    return 0;
}

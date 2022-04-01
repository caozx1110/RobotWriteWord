#pragma once
#include <vector>
#include <iostream>
using namespace std;

//可调参数，分支的最长最短值
#define MAX_BRANCH_LEN 20
#define MIN_BRANCH_LEN 2

class Pixel;
class Pixel
{
public:
    Pixel(int r = 0, int c = 0, bool isend = false, bool isnode = false);
    Pixel(const Pixel &p);
    Pixel &operator=(const Pixel &p);
    ~Pixel();
    //判断是否为端点
    void IsEndOrNode(vector<vector<bool>> *Map);
    //左上方点
    bool LeftUp(vector<vector<bool>> *Map, int *row, int *col);
    //左下方点
    bool LeftDown(vector<vector<bool>> *Map, int *row, int *col);
    //右上方点
    bool RightUp(vector<vector<bool>> *Map, int *row, int *col);
    //右下方点
    bool RightDown(vector<vector<bool>> *Map, int *row, int *col);
    //检索笔画
    static void FindStroke(vector<vector<bool>> *Map);

    int Row;     //行值
    int Col;     //列值
    bool IsEnd;  //是否为端点
    bool IsNode; //是否为节点
    static vector<Pixel> PixelList;
    static vector<Pixel> EndList;
    static vector<Pixel> NodeList;
    static vector<vector<Pixel>> StrokeList;
};

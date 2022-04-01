#include "Pixel.h"

vector<Pixel> Pixel::PixelList;
vector<Pixel> Pixel::EndList;
vector<Pixel> Pixel::NodeList;
vector<vector<Pixel>> Pixel::StrokeList;

Pixel::Pixel(int r, int c, bool isend, bool isnode) : Row(r), Col(c), IsEnd(isend), IsNode(isnode)
{
}

Pixel::Pixel(const Pixel &p)
{
    Row = p.Row;
    Col = p.Col;
    IsEnd = p.IsEnd;
    IsNode = p.IsNode;
}

Pixel &Pixel::operator=(const Pixel &p)
{
    if (this != &p)
    {
        this->Row = p.Row;
        this->Col = p.Col;
        this->IsEnd = p.IsEnd;
        this->IsNode = p.IsNode;
    }
    return *this;
}

Pixel::~Pixel()
{
}
//判断是否为端点或节点
void Pixel::IsEndOrNode(vector<vector<bool>> *Map)
{
    int Count = 0;
    for (int i = Row - 1; i <= Row + 1; i++)
    {
        for (int j = Col - 1; j <= Col + 1; j++)
        {
            if ((*Map)[i][j])
            {
                Count++;
            }
        }
    }
    if (Count == 2) //端点
    {
        EndList.push_back(*this);
        IsEnd = true;
        IsNode = false;
    }
    else if (Count == 4) //节点
    {
        NodeList.push_back(*this);
        IsEnd = false;
        IsNode = true;
    }
    else //普通点
    {
        IsEnd = false;
        IsNode = false;
    }
}
//左上
bool Pixel::LeftUp(vector<vector<bool>> *Map, int *row, int *col)
{
    //需要保证最外要留有一圈空白*******************************************！
    //另外判断顺序保证为顺时针（正左-左上-正上）
    if ((*Map)[Row][Col - 1])
    {
        *row = Row;
        *col = Col - 1;
        return true;
    }
    if ((*Map)[Row - 1][Col - 1])
    {
        *row = Row - 1;
        *col = Col - 1;
        return true;
    }
    if ((*Map)[Row - 1][Col])
    {
        *row = Row - 1;
        *col = Col;
        return true;
    }

    return false;
}
//左下
bool Pixel::LeftDown(vector<vector<bool>> *Map, int *row, int *col)
{
    if ((*Map)[Row + 1][Col])
    {
        *row = Row + 1;
        *col = Col;
        return true;
    }
    if ((*Map)[Row + 1][Col - 1])
    {
        *row = Row + 1;
        *col = Col - 1;
        return true;
    }
    if ((*Map)[Row][Col - 1])
    {
        *row = Row;
        *col = Col - 1;
        return true;
    }

    return false;
}
//右上
bool Pixel::RightUp(vector<vector<bool>> *Map, int *row, int *col)
{
    if ((*Map)[Row - 1][Col])
    {
        *row = Row - 1;
        *col = Col;
        return true;
    }
    if ((*Map)[Row - 1][Col + 1])
    {
        *row = Row - 1;
        *col = Col + 1;
        return true;
    }
    if ((*Map)[Row][Col + 1])
    {
        *row = Row;
        *col = Col + 1;
        return true;
    }
    return false;
}
//右下
bool Pixel::RightDown(vector<vector<bool>> *Map, int *row, int *col)
{
    if ((*Map)[Row][Col + 1])
    {
        *row = Row;
        *col = Col + 1;
        return true;
    }
    if ((*Map)[Row + 1][Col + 1])
    {
        *row = Row + 1;
        *col = Col + 1;
        return true;
    }
    if ((*Map)[Row + 1][Col])
    {
        *row = Row + 1;
        *col = Col;
        return true;
    }
    return false;
}
//检索笔画
void Pixel::FindStroke(vector<vector<bool>> *Map)
{
    Pixel temp;
    int tempRow;
    int tempCol;
    int IniSize = NodeList.size();
    for (int i = 0; i < IniSize; i++)
    {
        //左上方向延申
        temp = NodeList[i];
        StrokeList.push_back(vector<Pixel>());
        StrokeList.back().push_back(temp);
        while (temp.LeftUp(Map, &tempRow, &tempCol))
        {
            if (temp.IsNode)
            {
                StrokeList.pop_back();
                break;
            }

            //更新temp
            temp = Pixel(tempRow, tempCol);
            temp.IsEndOrNode(Map);
            // push temp
            StrokeList.back().push_back(temp);
        }
        if (!temp.IsEnd && !temp.IsNode) //如果是Node，已经删过一次了
        {
            StrokeList.pop_back();
        }

        //左下方向延申
        temp = NodeList[i];
        StrokeList.push_back(vector<Pixel>());
        StrokeList.back().push_back(temp);
        while (temp.LeftDown(Map, &tempRow, &tempCol))
        {
            if (temp.IsNode)
            {
                StrokeList.pop_back();
                break;
            }

            //更新temp
            temp = Pixel(tempRow, tempCol);
            temp.IsEndOrNode(Map);
            // push temp
            StrokeList.back().push_back(temp);
        }
        if (!temp.IsEnd && !temp.IsNode) //如果是Node，已经删过一次了
        {
            StrokeList.pop_back();
        }

        //右上方向延申
        temp = NodeList[i];
        StrokeList.push_back(vector<Pixel>());
        StrokeList.back().push_back(temp);
        while (temp.RightUp(Map, &tempRow, &tempCol))
        {
            if (temp.IsNode)
            {
                StrokeList.pop_back();
                break;
            }

            //更新temp
            temp = Pixel(tempRow, tempCol);
            temp.IsEndOrNode(Map);
            // push temp
            StrokeList.back().push_back(temp);
        }
        if (!temp.IsEnd && !temp.IsNode) //如果是Node，已经删过一次了
        {
            StrokeList.pop_back();
        }

        //右下方向延申
        temp = NodeList[i];
        StrokeList.push_back(vector<Pixel>());
        StrokeList.back().push_back(temp);
        while (temp.RightDown(Map, &tempRow, &tempCol))
        {
            if (temp.IsNode)
            {
                StrokeList.pop_back();
                break;
            }

            //更新temp
            temp = Pixel(tempRow, tempCol);
            temp.IsEndOrNode(Map);
            // push temp
            StrokeList.back().push_back(temp);
        }
        if (!temp.IsEnd && !temp.IsNode) //如果是Node，已经删过一次了
        {
            StrokeList.pop_back();
        }
    }
}

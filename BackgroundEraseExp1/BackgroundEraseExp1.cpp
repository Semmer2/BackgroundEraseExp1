// BackgroundEraseExp1.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include"iostream"
#include<opencv2\opencv.hpp>
using namespace std;
using namespace cv;


int main()
{
	Mat image = imread("House.jpg");
	imshow("image",image);
	system("pause");
    return 0;
}


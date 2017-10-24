// BackgroundEraseExp1.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include"iostream"
#include<opencv2\opencv.hpp>
#include"EraseBGD.h"
using namespace std;
using namespace cv;


int main()
{
	Mat image = imread("House.jpg",CV_LOAD_IMAGE_UNCHANGED);
	Mat imageGray = imread("House.jpg", CV_LOAD_IMAGE_GRAYSCALE);
	//imshow("image",image);
	imshow("imageGrayscale", imageGray);
	//EraseBGDhist(imageGray);

	BlurAndSharp(imageGray);

	Mat BiImage = AutoToBiImage(imageGray);
	OptimizeRegion(BiImage);
	waitKey(0);
    return 0;
}


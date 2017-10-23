// BackgroundEraseExp1.cpp : 定义控制台应用程序的入口点。
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
	Mat imageGray = imread("House3.jpg", CV_LOAD_IMAGE_GRAYSCALE);
	//imshow("image",image);
	imshow("imageGrayscale", imageGray);
	EraseBGDhist(imageGray);
	Mat imageBil = image;
	bilateralFilter(image, imageBil, 25, 25 * 2, 25 / 2);
	imshow("bilFilter", image);
	waitKey(0);
    return 0;
}


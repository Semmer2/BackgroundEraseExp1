#include "stdafx.h"
#include"iostream"
#include<opencv2\opencv.hpp>
using namespace std;
using namespace cv;

void EraseBGDhist(Mat image)
{
	const int channels[1] = { 0 };
	const int histSize[1] = { 256 };
	float hranges[2] = { 0,255 };
	const float* ranges[1] = { hranges };
	MatND hist;
	calcHist(&image, 1, channels, Mat(), hist, 1, histSize, ranges);

	double maxVal = 0;
	double minVal = 0;
	double midVal = 0;

	int low = 0, high = 0;

	//找到直方图中的最大值和最小值
	minMaxLoc(hist, &minVal, &maxVal, 0, 0);
	midVal = (maxVal + minVal) / 20;
	printf("%f,%f\n", maxVal, minVal);
	int histSizer = hist.rows;
	Mat histImg(histSizer + 20, histSizer + 20, CV_8UC3, Scalar(180, 180, 180));
	rectangle(histImg, Rect(Point(10, 10), Point(histSizer + 10, histSizer + 10)), Scalar(255, 255, 255), -1, 8);

	for (int h = 0; h<histSizer; h++)
	{
		float histVal = hist.at<float>(h);
		int length = static_cast<int>(histVal*histSizer / maxVal);

		//寻找上下限
		if (histVal > midVal&&low == 0)
			low = h;
		if (histVal < midVal&&low != 0 && high == 0)
			high = h;
		line(histImg, Point(h + 10, histSizer + 10), Point(h + 10, histSizer - length + 10), Scalar(h, h, h));
	}

	namedWindow("hist", CV_WINDOW_AUTOSIZE);
	imshow("hist", histImg);
	cout << "low: " << low << " high: " << high << endl;

	Mat result = image;
	MatIterator_<uchar> it, end;
	for (it = result.begin<uchar>(), end = result.end<uchar>(); it != end; it++)
		if (*it <= high&&*it >= low - 1)//low-1是为了让其覆盖更全面，否则出现某一单个值过高而无法准确去除的情况
		{
			*it = 0;
		}
	result = 255 - result;
	imshow("result", result);

	waitKey(0);
}


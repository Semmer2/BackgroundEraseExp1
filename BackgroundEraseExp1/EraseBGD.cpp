#include "stdafx.h"
#include"iostream"
#include<opencv2\opencv.hpp>
using namespace std;
using namespace cv;

void RemoveSmallRegion(Mat &Src, Mat &Dst, int AreaLimit, int CheckMode, int NeihborMode);

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
	//显示直方图
	//imshow("hist", histImg);
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

	//waitKey(0);
}

Mat BlurAndSharp(Mat imageGray)
{
	Mat imageBil = Mat(imageGray.rows, imageGray.cols, CV_8UC1);
	bilateralFilter(imageGray, imageBil, 25, 25 * 2, 25 / 2);
	imshow("bilFilter", imageBil);
	return imageBil;
}

Mat AutoToBiImage(Mat imageGray)
{
	Mat AutoBiImage;
	int blockSize = 7;
	int constValue = 11;
	adaptiveThreshold(imageGray, AutoBiImage, 255, CV_ADAPTIVE_THRESH_MEAN_C, CV_THRESH_BINARY_INV, blockSize, constValue);
	imshow("AutoBiImage", AutoBiImage);
	return AutoBiImage;
}

Mat OptimizeRegion(Mat BiImage)
{
	Mat OptimizedImage = Mat::zeros(BiImage.rows, BiImage.cols, CV_8UC1);;
	RemoveSmallRegion(BiImage, OptimizedImage, 50, 1, 0);
	morphologyEx(OptimizedImage, OptimizedImage, MORPH_OPEN, getStructuringElement(MORPH_RECT, Size(3, 3)));
	imshow("OptimizedImage", OptimizedImage);
	return OptimizedImage;
}

//移除小连通区域
void RemoveSmallRegion(Mat &Src, Mat &Dst, int AreaLimit, int CheckMode, int NeihborMode)
{
	int RemoveCount = 0;
	//新建一幅标签图像初始化为0像素点，为了记录每个像素点检验状态的标签，0代表未检查，1代表正在检查,2代表检查不合格（需要反转颜色），3代表检查合格或不需检查 
	//初始化的图像全部为0，未检查
	Mat PointLabel = Mat::zeros(Src.size(), CV_8UC1);
	if (CheckMode == 1)//去除小连通区域的白色点
	{
		cout << "去除小连通域.";
		for (int i = 0; i < Src.rows; i++)
		{
			for (int j = 0; j < Src.cols; j++)
			{
				if (Src.at<uchar>(i, j) < 10)
				{
					PointLabel.at<uchar>(i, j) = 3;//将背景黑色点标记为合格，像素为3
				}
			}
		}
	}
	else//去除孔洞，黑色点像素
	{
		cout << "去除孔洞";
		for (int i = 0; i < Src.rows; i++)
		{
			for (int j = 0; j < Src.cols; j++)
			{
				if (Src.at<uchar>(i, j) > 10)
				{
					PointLabel.at<uchar>(i, j) = 3;//如果原图是白色区域，标记为合格，像素为3
				}
			}
		}
	}


	vector<Point2i>NeihborPos;//将邻域压进容器
	NeihborPos.push_back(Point2i(-1, 0));
	NeihborPos.push_back(Point2i(1, 0));
	NeihborPos.push_back(Point2i(0, -1));
	NeihborPos.push_back(Point2i(0, 1));
	if (NeihborMode == 1)
	{
		cout << "Neighbor mode: 8邻域." << endl;
		NeihborPos.push_back(Point2i(-1, -1));
		NeihborPos.push_back(Point2i(-1, 1));
		NeihborPos.push_back(Point2i(1, -1));
		NeihborPos.push_back(Point2i(1, 1));
	}
	else cout << "Neighbor mode: 4邻域." << endl;
	int NeihborCount = 4 + 4 * NeihborMode;
	int CurrX = 0, CurrY = 0;
	//开始检测
	for (int i = 0; i < Src.rows; i++)
	{
		for (int j = 0; j < Src.cols; j++)
		{
			if (PointLabel.at<uchar>(i, j) == 0)//标签图像像素点为0，表示还未检查的不合格点
			{   //开始检查
				vector<Point2i>GrowBuffer;//记录检查像素点的个数
				GrowBuffer.push_back(Point2i(j, i));
				PointLabel.at<uchar>(i, j) = 1;//标记为正在检查
				int CheckResult = 0;


				for (int z = 0; z < GrowBuffer.size(); z++)
				{
					for (int q = 0; q < NeihborCount; q++)
					{
						CurrX = GrowBuffer.at(z).x + NeihborPos.at(q).x;
						CurrY = GrowBuffer.at(z).y + NeihborPos.at(q).y;
						if (CurrX >= 0 && CurrX<Src.cols&&CurrY >= 0 && CurrY<Src.rows)  //防止越界  
						{
							if (PointLabel.at<uchar>(CurrY, CurrX) == 0)
							{
								GrowBuffer.push_back(Point2i(CurrX, CurrY));  //邻域点加入buffer  
								PointLabel.at<uchar>(CurrY, CurrX) = 1;           //更新邻域点的检查标签，避免重复检查  
							}
						}
					}
				}
				if (GrowBuffer.size()>AreaLimit) //判断结果（是否超出限定的大小），1为未超出，2为超出  
					CheckResult = 2;
				else
				{
					CheckResult = 1;
					RemoveCount++;//记录有多少区域被去除
				}


				for (int z = 0; z < GrowBuffer.size(); z++)
				{
					CurrX = GrowBuffer.at(z).x;
					CurrY = GrowBuffer.at(z).y;
					PointLabel.at<uchar>(CurrY, CurrX) += CheckResult;//标记不合格的像素点，像素值为2
				}
				//********结束该点处的检查**********  


			}
		}


	}


	CheckMode = 255 * (1 - CheckMode);
	//开始反转面积过小的区域  
	for (int i = 0; i < Src.rows; ++i)
	{
		for (int j = 0; j < Src.cols; ++j)
		{
			if (PointLabel.at<uchar>(i, j) == 2)
			{
				Dst.at<uchar>(i, j) = CheckMode;
			}
			else if (PointLabel.at<uchar>(i, j) == 3)
			{
				Dst.at<uchar>(i, j) = Src.at<uchar>(i, j);

			}
		}
	}
	cout << RemoveCount << " objects removed." << endl;
}
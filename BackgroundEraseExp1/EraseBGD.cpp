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

	//�ҵ�ֱ��ͼ�е����ֵ����Сֵ
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

		//Ѱ��������
		if (histVal > midVal&&low == 0)
			low = h;
		if (histVal < midVal&&low != 0 && high == 0)
			high = h;
		line(histImg, Point(h + 10, histSizer + 10), Point(h + 10, histSizer - length + 10), Scalar(h, h, h));
	}

	namedWindow("hist", CV_WINDOW_AUTOSIZE);
	//��ʾֱ��ͼ
	//imshow("hist", histImg);
	cout << "low: " << low << " high: " << high << endl;

	Mat result = image;
	MatIterator_<uchar> it, end;
	for (it = result.begin<uchar>(), end = result.end<uchar>(); it != end; it++)
		if (*it <= high&&*it >= low - 1)//low-1��Ϊ�����串�Ǹ�ȫ�棬�������ĳһ����ֵ���߶��޷�׼ȷȥ�������
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

//�Ƴ�С��ͨ����
void RemoveSmallRegion(Mat &Src, Mat &Dst, int AreaLimit, int CheckMode, int NeihborMode)
{
	int RemoveCount = 0;
	//�½�һ����ǩͼ���ʼ��Ϊ0���ص㣬Ϊ�˼�¼ÿ�����ص����״̬�ı�ǩ��0����δ��飬1�������ڼ��,2�����鲻�ϸ���Ҫ��ת��ɫ����3������ϸ������ 
	//��ʼ����ͼ��ȫ��Ϊ0��δ���
	Mat PointLabel = Mat::zeros(Src.size(), CV_8UC1);
	if (CheckMode == 1)//ȥ��С��ͨ����İ�ɫ��
	{
		cout << "ȥ��С��ͨ��.";
		for (int i = 0; i < Src.rows; i++)
		{
			for (int j = 0; j < Src.cols; j++)
			{
				if (Src.at<uchar>(i, j) < 10)
				{
					PointLabel.at<uchar>(i, j) = 3;//��������ɫ����Ϊ�ϸ�����Ϊ3
				}
			}
		}
	}
	else//ȥ���׶�����ɫ������
	{
		cout << "ȥ���׶�";
		for (int i = 0; i < Src.rows; i++)
		{
			for (int j = 0; j < Src.cols; j++)
			{
				if (Src.at<uchar>(i, j) > 10)
				{
					PointLabel.at<uchar>(i, j) = 3;//���ԭͼ�ǰ�ɫ���򣬱��Ϊ�ϸ�����Ϊ3
				}
			}
		}
	}


	vector<Point2i>NeihborPos;//������ѹ������
	NeihborPos.push_back(Point2i(-1, 0));
	NeihborPos.push_back(Point2i(1, 0));
	NeihborPos.push_back(Point2i(0, -1));
	NeihborPos.push_back(Point2i(0, 1));
	if (NeihborMode == 1)
	{
		cout << "Neighbor mode: 8����." << endl;
		NeihborPos.push_back(Point2i(-1, -1));
		NeihborPos.push_back(Point2i(-1, 1));
		NeihborPos.push_back(Point2i(1, -1));
		NeihborPos.push_back(Point2i(1, 1));
	}
	else cout << "Neighbor mode: 4����." << endl;
	int NeihborCount = 4 + 4 * NeihborMode;
	int CurrX = 0, CurrY = 0;
	//��ʼ���
	for (int i = 0; i < Src.rows; i++)
	{
		for (int j = 0; j < Src.cols; j++)
		{
			if (PointLabel.at<uchar>(i, j) == 0)//��ǩͼ�����ص�Ϊ0����ʾ��δ���Ĳ��ϸ��
			{   //��ʼ���
				vector<Point2i>GrowBuffer;//��¼������ص�ĸ���
				GrowBuffer.push_back(Point2i(j, i));
				PointLabel.at<uchar>(i, j) = 1;//���Ϊ���ڼ��
				int CheckResult = 0;


				for (int z = 0; z < GrowBuffer.size(); z++)
				{
					for (int q = 0; q < NeihborCount; q++)
					{
						CurrX = GrowBuffer.at(z).x + NeihborPos.at(q).x;
						CurrY = GrowBuffer.at(z).y + NeihborPos.at(q).y;
						if (CurrX >= 0 && CurrX<Src.cols&&CurrY >= 0 && CurrY<Src.rows)  //��ֹԽ��  
						{
							if (PointLabel.at<uchar>(CurrY, CurrX) == 0)
							{
								GrowBuffer.push_back(Point2i(CurrX, CurrY));  //��������buffer  
								PointLabel.at<uchar>(CurrY, CurrX) = 1;           //���������ļ���ǩ�������ظ����  
							}
						}
					}
				}
				if (GrowBuffer.size()>AreaLimit) //�жϽ�����Ƿ񳬳��޶��Ĵ�С����1Ϊδ������2Ϊ����  
					CheckResult = 2;
				else
				{
					CheckResult = 1;
					RemoveCount++;//��¼�ж�������ȥ��
				}


				for (int z = 0; z < GrowBuffer.size(); z++)
				{
					CurrX = GrowBuffer.at(z).x;
					CurrY = GrowBuffer.at(z).y;
					PointLabel.at<uchar>(CurrY, CurrX) += CheckResult;//��ǲ��ϸ�����ص㣬����ֵΪ2
				}
				//********�����õ㴦�ļ��**********  


			}
		}


	}


	CheckMode = 255 * (1 - CheckMode);
	//��ʼ��ת�����С������  
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
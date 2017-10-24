#include "stdafx.h"
#include"iostream"
#include<opencv2\opencv.hpp>
using namespace std;
using namespace cv; 
#pragma once

void EraseBGDhist(Mat image);

Mat AutoToBiImage(Mat imageGray);

Mat BlurAndSharp(Mat imageGray);

Mat OptimizeRegion(Mat BiImage);
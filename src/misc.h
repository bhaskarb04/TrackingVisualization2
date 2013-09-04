#pragma once

#include <string>
#include <stdio.h>
#include <iostream>
#include <opencv\cv.h>
#include <opencv\highgui.h>
#include <utility>
#include <vector>
#include <assert.h>
#include <deque>

using namespace cv;
using namespace std;


struct hypo{
	double alpha,beta,theta;
	double xc,yc;
	int age;
	Mat hist;
	cv::Point v;
	double area;
};
struct HPoint{
	cv::Point p;
	vector<int> label;
};

enum condition{NOTHING,BREAK,JOIN,NEW};

#define PI 3.1415926535
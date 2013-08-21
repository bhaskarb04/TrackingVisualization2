#pragma once
#include "Hypothesis.h"
#include "Visualizer.h"
#include <opencv\cv.h>
#include <highgui.h>
#include <vector>
using namespace std;
using namespace cv;

#define MORPH_SIZE 7
#define MINCSIZE 0.005
class Process{
private:
	
	vector<cv::Mat> images;
	int num_of_frames,width,height;
	Hypothesis *hype;
	double maximum;
	vector<vector<vector<cv::Point>>> listofcontours;
	Visualizer *visualizer;
	
	void clean_image(Mat &m);
	void checkmax(Mat);
	void clean_noise_contours(vector<vector<cv::Point> >&);
public:
	Process(float **d,int num_of_frames,int w,int h);
	~Process();
	void make_contours();

};
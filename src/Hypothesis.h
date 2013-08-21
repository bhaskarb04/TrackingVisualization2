#pragma once
#include <string>
#include <stdio.h>
#include <iostream>
#include <opencv\cv.h>
#include <opencv\highgui.h>
#include <utility>
#include <assert.h>

using namespace cv;
using namespace std;

#define PI 3.1415926535
#define AGE_MAX 5
#define AREA_THRESH 0.9
#define D_MAX 20

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

class Hypothesis{
private:
	vector<hypo>listh;
	vector<vector<vector<cv::Point>>> listcontours;
	vector<Mat>images;
	double histmax;
	int nolabels;
	vector<pair<int,int>>links;
	vector<vector<pair<int,int>>> framelinks;
	vector<vector<int>>framecondition;
	vector<vector<int>>joinvec;
	vector<vector<pair<int,int>>>btob;

	hypo create_hypo(vector<cv::Point>,Mat,int);
	bool check_hypos(vector<HPoint>&,int);
	bool check_hypo(HPoint&,bool);
	double getD(HPoint p,int listhno);
	double compare_ellipses(hypo h1,hypo h2);
	void update_hypo(bool,hypo,int);
	void reduce_age();
	void check_blob_count(int,vector<hypo>&);
	void hypos_broke(vector<hypo>&);
	void hypos_join(vector<hypo>&);
	void cleanupandhousekeeping(vector<hypo>&,vector<int>,vector<int>,condition);
	void draw_all(int frameno,vector<hypo> hv);
	void draw_with_one(int frameno, hypo h);
	void show_analysis_time(float t);
	bool near_the_edge(hypo h);
public:
	Hypothesis(vector<vector<vector<cv::Point>>>,double,vector<Mat>);
	~Hypothesis();
	void track();
	vector<vector<pair<int,int>>> get_framelinks(){return framelinks;}
	vector<vector<int>> get_conditions(){return framecondition;}
	vector<vector<int>> get_joinvec(){return joinvec;}

};
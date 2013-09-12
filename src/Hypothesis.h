#pragma once
#include "misc.h"

using namespace cv;
using namespace std;

#define AGE_MAX 5
#define AREA_THRESH 0.9
#define D_MAX 20

class Hypothesis{
private:
	vector<hypo>listh;
	vector<vector<vector<cv::Point>>> listcontours;
	vector<Mat>images;
	double histmax,histmin;
	int nolabels;
	int maxnolabels;
	vector<pair<int,int>>links;
	vector<pair<int,int>>bloblinks;
	vector<vector<pair<int,int>>> framelinks;
	vector<vector<int>>framecondition;
	deque<vector<int>>joinvec;
	vector<vector<pair<int,int>>>btob;
	vector<vector<vector<HPoint>>> all_points;
	vector<vector<hypo>> all_hypo;
	vector<vector<int>>gonelist;
	vector<pair<int,int>>nodelist;
	vector<pair<int,int>>adjlist;
	vector<vector<vector<int>>>framecontcolor;
	vector<pair<int,int>> repeat;
	vector<int>colorh;
	vector<vector<int>>all_color;
	vector<bool**>valid;
	vector<cv::Mat>labels;
	vector<map<int,cv::Point3d>>centres;

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
	void modify_points(vector<int>remove,vector<int>place,condition c);
	void modify_points_break(vector<int>remove,vector<int>place);
	void make_adjlist();
	bool find_in_nodelist(int frameno,int prev,int& n1);
	int find_starting_point(int);
	bool find_prev(int&,int&);
	bool find_next(int&,int&);
	void check_for_repeat(int,int,bool);
	void make_colors();
	void check_localpos();
	void update_all(vector<bool>matches,vector<hypo>framehypo);
public:
	Hypothesis(vector<vector<vector<cv::Point>>>,double,vector<Mat>);
	~Hypothesis();
	void track();
	vector<vector<pair<int,int>>> get_framelinks(){return framelinks;}
	vector<vector<int>> get_conditions(){return framecondition;}
	//vector<vector<int>> get_joinvec(){return joinvec;}

	vector<pair<int,int>> get_nodelist(){return nodelist;}
	vector<pair<int,int>> get_adjacencylist(){return adjlist;}
	vector<vector<hypo>> get_hypolist(){return all_hypo;}
	vector<vector<vector<int>>> get_fcc(){return framecontcolor;}
	int get_nocolors(){return maxnolabels;}
	vector<vector<vector<HPoint>>> get_allpoints(){return all_points;}
	vector<bool**> get_validity(){return valid;}
	vector<cv::Mat> get_labels(){return labels;}
	vector<map<int,cv::Point3d>> get_centres(){return centres;}

};
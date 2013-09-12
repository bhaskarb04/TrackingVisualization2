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
	double maximum,minimum;
	vector<vector<vector<cv::Point>>> listofcontours;
	Visualizer *visualizer;
	
	void clean_image(Mat &m);
	void checkmax(Mat);
	void clean_noise_contours(vector<vector<cv::Point> >&);
public:
	Process(float **d,int num_of_frames,int w,int h);
	~Process();
	void make_contours();
	
	//Movement functions
	//Particle movements
	void move_particle_xplus(){visualizer->move_particle_xplus();}
	void move_particle_xminus(){visualizer->move_particle_xminus();}
	void move_particle_yplus(){visualizer->move_particle_yplus();}
	void move_particle_yminus(){visualizer->move_particle_yminus();}
	void move_particle_zplus(){visualizer->move_particle_zplus();}
	void move_particle_zminus(){visualizer->move_particle_zminus();}
	void scroll_particle(){visualizer->scroll_particle();}
	void move_particle_scaleplus(){visualizer->move_particle_scaleplus();}
	void move_particle_scaleminus(){visualizer->move_particle_scaleminus();}

	//Target movements
	void scroll_target(){visualizer->scroll_target();}

};
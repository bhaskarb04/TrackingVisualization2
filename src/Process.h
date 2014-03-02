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
	osg::ref_ptr<osg::Group> return_root(){return visualizer->return_root();}
	void view(){visualizer->view();}
	
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
	//Particle transform
	void showspheres(bool toggle){visualizer->showspheres(toggle);}
	//Bg toggling
	void togglebg(bool toggle){visualizer->togglebg(toggle);}

	//Target movements
	void scroll_target(){visualizer->scroll_target();}

	//Model functions
	void load_model(string modelname){visualizer->tractordraw(modelname);}
	void translate_model_xplus(){visualizer->translate_model_xplus();}
	void translate_model_xminus(){visualizer->translate_model_xminus();}
	void translate_model_yplus(){visualizer->translate_model_yplus();}
	void translate_model_yminus(){visualizer->translate_model_yminus();}
	void translate_model_zplus(){visualizer->translate_model_zplus();}
	void translate_model_zminus(){visualizer->translate_model_zminus();}
	void rotate_model_xplus(){visualizer->rotate_model_xplus();}
	void rotate_model_xminus(){visualizer->rotate_model_xminus();}
	void rotate_model_yplus(){visualizer->rotate_model_yplus();}
	void rotate_model_yminus(){visualizer->rotate_model_yminus();}
	void rotate_model_zplus(){visualizer->rotate_model_zplus();}
	void rotate_model_zminus(){visualizer->rotate_model_zminus();}

	void translate_particle_x(float val){visualizer->move_particle_x(val);}
	void translate_particle_y(float val){visualizer->move_particle_y(val);}
	void translate_particle_z(float val){visualizer->move_particle_z(val);}

	void togglemodel(bool tg){visualizer->togglemodel(tg);}
	void save_model(){visualizer->save_model();}
	void load_model(){visualizer->load_model();}
};
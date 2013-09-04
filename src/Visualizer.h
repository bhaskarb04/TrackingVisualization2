#pragma once

#include <osg/Geometry>
#include <osg/Geode>
#include <osg/Camera>
#include <osg/Point>
#include <osg/MatrixTransform>
#include <osg/PositionAttitudeTransform>
#include <osg/Shape>
#include <osg/ShapeDrawable>
#include <osg/Image>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <osgGA/TrackballManipulator>
#include <osgGA/CameraManipulator>
#include <osgUtil/SmoothingVisitor>
#include <osgUtil\DelaunayTriangulator>
#include <osgViewer/Viewer>

#include <opencv\cv.h>

#include <utility>
#include <vector>

#include "misc.h"
using namespace std;
using namespace cv;
using namespace osg;

#define SLOW_DOWN 2

class VisualizerCallback;

class Visualizer
{
	osg::ref_ptr<osg::Group> root;
	osg::ref_ptr<osg::MatrixTransform> transform;
	osg::ref_ptr<osg::MatrixTransform> particletransform;
	VisualizerCallback *vc;
	vector<osg::Vec3Array*> vertices;
	vector<osg::DrawElementsUInt*> inds;
	vector<osg::Vec3Array*> verts;
	float **data;
	int nframes;
	int width,height,totalcolors;

	vector<vector<vector<cv::Point>>> listcontours;
	vector<vector<pair<int,int>>> framelinks;
	vector<vector<osg::Vec3Array*>> vertices_all;
	vector<vector<osg::DrawElementsUInt*>> inds_all;
	vector<osg::ref_ptr<osg::Geode>> geodes;
	vector<vector<osg::Vec4>> colors;
	vector<vector<int>> conditions;
	vector<vector<int>> joinvec;
	vector<vector<vector<int>>>framecontcolor;
	vector<vector<osg::Vec4Array*>>colorarray;
	
	osg::ref_ptr<osg::Geode> bgdraw();
	osg::ref_ptr<osg::PositionAttitudeTransform> tractordraw();
	osg::ref_ptr<osg::Geode> drawAxes();
	osg::ref_ptr<osg::LightSource> get_lightsource();
	void setscene();
	void create_vertices();
	void make_all_vertices();
	osg::Vec3 convert_deere_geom(float x,float y,float z);
	bool check_if_valid(vector<cv::Point> contour,int x1,int y1,int x2,int y2,int x3,int y3,float f1,float f2, float f3);
	bool check_xy_valid(int,int);
	void show_visual_analysis_time(float t);
	osg::Vec4 make_new_color();
	void make_color_array();
	void make_vertices(int start,int end,int i,vector<vector<cv::Point>> contours,cv::Rect r,osg::Vec3Array*& vdata,osg::DrawElementsUInt*& idata);
	int find_bin(int n,int x);
public:
	Visualizer(float **d,int num_of_frames,int w,int h);
	~Visualizer(void);
	void add_contours(vector<vector<vector<cv::Point>>>);
	void add_links(vector<vector<pair<int,int>>>);
	void add_conditions(vector<vector<int>>);
	void add_joinvec(vector<vector<int>>);
	void add_fcc(vector<vector<vector<int>>>);
	void add_nocolors(int);
	void update_vertices(int);
	void view();

};

class VisualizerCallback : public osg::NodeCallback{

private:
	int frame;
	int nframes;
	Visualizer *parent;
	int slow;
public:
	VisualizerCallback(int,Visualizer*);
	~VisualizerCallback();
    virtual void operator()(osg::Node* node, osg::NodeVisitor* nv);
};


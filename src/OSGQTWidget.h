#pragma once
#include <QtCore/QTimer>
#include <QtGui/QApplication>
#include <QtGui/QGridLayout>
#include <osgViewer/CompositeViewer>
#include <osgViewer/ViewerEventHandlers>
#include <osgGA/TrackballManipulator>
#include <osgDB/ReadFile>
#include <osg/PositionAttitudeTransform>
#include <osgQt/GraphicsWindowQt>
#include <QObject>
#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <string.h>
using namespace std;

#define TIMER_ 33

class ViewerWidget : public QWidget, public osgViewer::CompositeViewer
{
	Q_OBJECT
public:
	ViewerWidget(QWidget * parent);
	osgViewer::View* view;
	void draw_data(osg::Node*);
	void delete_data();
	void draw_model(string);
	void delete_model();
	QWidget* addViewWidget( osgQt::GraphicsWindowQt *, osg::Node* scene );
	osgQt::GraphicsWindowQt* createGraphicsWindow( int x, int y, int w, int h);
	osg::Camera* createCamera( int x, int y, int w, int h, const std::string& name="", bool windowDecoration=false );
	virtual void paintEvent( QPaintEvent* event ){ frame(); }
	
	void translate_model_x(float);
	void translate_model_y(float);
	void translate_model_z(float);
	void rotate_model_x(float);
	void rotate_model_y(float);
	void rotate_model_z(float);
	void switch_model(bool);
	void translate_data_x(float);
	void translate_data_y(float);
	void translate_data_z(float);
protected:
    QTimer _timer;
	QGridLayout* grid;
	osg::ref_ptr<osg::Group> root;
	osg::ref_ptr<osg::PositionAttitudeTransform> datapat;
	osg::ref_ptr<osg::PositionAttitudeTransform> modelpat;
	bool home;
};
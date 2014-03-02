#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "rawReader.h"
#include "Visualizer.h"
#include "Process.h"
#include <QMainWindow>
#include <QFileDialog>
#include <QMessageBox>
#include <QTimer>
#include <QProgressBar>
#include <QGLWidget>
#include <osgViewer/CompositeViewer>
#include <osgQt/GraphicsWindowQt>
#include <opencv/cv.h>
#include <string>
using namespace std;

#define RAWWIDTH 576
#define RAWHEIGHT 576

#define DEFAULT_TIMER_INTERVAL 33
#define PICFORMAT ".bmp"
#define SLIDERSTEP 4

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::MainWindow *ui;
	string path;
	string origpath;
	string cleanedpath;
	string contpath;
	string immpath;
	string modelpath;
	QImage *list;
	QTimer *timer;
	Process *process;
	int timer_interval,current_frame,frame_end;
	bool sceneset;
	bool modelloaded;
	QString modelfileName;
	cv::Point3f datatranslation;
	cv::Point3f modeltranslation;
	cv::Point3f modelrotation;
	cv::Point3f tempmodelmovement;
	bool slideron;
private slots:;
	//Load and remove buttons
	void load_button_click();
	void load_model_click();
	void delete_button_click();
	void delete_model_click();

	//Data checkboxes
	void toggleshowspheres(int toggle){process->showspheres(bool(toggle));}
	void togglebackground(int toggle){process->togglebg(bool(toggle));}	
	void togglemodel(int toggle);

	//Data movements
	void xslidervalue(int);
	void yslidervalue(int);
	void zslidervalue(int);

	//Model movements
	void modelxslidervalue(int);
	void modelyslidervalue(int);
	void modelzslidervalue(int);
	void modelrotxslidervalue(int);
	void modelrotyslidervalue(int);
	void modelrotzslidervalue(int);

	//Snapback effect
	void snapback();

};

class MyGLDrawer : public QGLWidget
 {
     Q_OBJECT        // must include this if you use Qt signals/slots

 public:
     MyGLDrawer(QWidget *parent)
         : QGLWidget(parent) {}

 protected:
	 GLuint num;
	 QImage buffer;
     void initializeGL()
     {
		// Set up the rendering context, define display lists etc.:
		glClearColor(1.0, 1.0, 1.0, 0.0);
		/*glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glEnable(GL_DEPTH_TEST);
		glOrtho(-1,1,-1,1,-1,1);
		glGenTextures(1,&num);*/
     }
	 void paintGL();
 public:
	 void drawImage(QImage& image);

 };

#endif // MAINWINDOW_H

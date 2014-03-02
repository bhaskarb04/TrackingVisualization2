#include <QtGui/QApplication>
#include "mainwindow.h"

#define RAWWIDTH 576
#define RAWHEIGHT 576
#define DATAPATH "E:/Dropbox/Cotton_814/3break_2000_9fringes"
#define START_IMAGE 1
#define END_IMAGE 26
#define NO_IMAGES END_IMAGE-START_IMAGE+1
void make_video(vector<cv::Mat>lhs,vector<cv::Mat>rhs,string vidname,string path);
int main(int argc, char**argv){

	QApplication a(argc, argv);
	MainWindow w;
	//w.setStyleSheet();
	w.show();
	int b=a.exec();
	cout<<endl<<b<<endl;
	return b;
//	osg::Node* blah= osgDB::readNodeFile("cow.osg");
	//rawReader reader;
	//reader.readfile(DATAPATH,START_IMAGE,END_IMAGE,RAWWIDTH,RAWHEIGHT);
	//Process p(reader.data,NO_IMAGES,RAWWIDTH,RAWHEIGHT);
	//vector<Mat> newimages;
	//for(int i=0;i<NO_IMAGES;i++){
	//	cv::Mat mm(RAWHEIGHT,RAWWIDTH,CV_32FC1,reader.data[i]);
	//	Mat m;
	//	mm.copyTo(m);
	//	m=(m)*(255.0);
	//	m.convertTo(m,CV_8UC1);
	//	cv::flip(m,m,0);
	//	char num[5];
	//	sprintf(num,"%03d",i);
	//	string s= string("RawImage")+string(num)+string(".png");
	//	//m.copyTo(p.images[i]);
	//	cv::cvtColor(m,m,CV_GRAY2RGB);
	//	//imwrite(s,m);
	//	imshow("jk",m);
	//	cv::waitKey(10);
	//	newimages.push_back(m);
	//}
	//p.make_contours();
	//Visualizer v(reader.data,NO_IMAGES,RAWWIDTH,RAWHEIGHT);
	
	//make_video(newimages,p.visualizer->osgVideo,"DeereNewViz5.avi","./");
	return 0;
}

 void make_video(vector<cv::Mat>lhs,vector<cv::Mat>rhs,string vidname,string path)
 {
	 if(lhs.size()==0 || rhs.size()==0 || (lhs.size() > rhs.size()))
		 return;
	 cv::Size imgsize=lhs[0].size();
	 cv::Mat vidimg(imgsize.height,2*imgsize.width,CV_8UC3);
	 cv::VideoWriter video(vidname, CV_FOURCC('D','I','V','X'), 30, vidimg.size(), true);
	 for(unsigned int i=0,j=0;i<rhs.size();i++,j++)
	 {
		 j=(j>=lhs.size()?0:j);
		 cv::Mat l=lhs[j];
		 cv::Mat r=rhs[i];
		 cv::Mat lroi=vidimg(cv::Rect(0,0,imgsize.width,imgsize.height));
		 cv::Mat rroi=vidimg(cv::Rect(imgsize.width,0,imgsize.width,imgsize.height));
		 cv::resize(rhs[i],r,imgsize);
		 l.copyTo(lroi);
		 r.copyTo(rroi);;
		 cv::imshow("finalvideo",vidimg);
		 cv::waitKey(100);
		 video<<vidimg;
		 video<<vidimg;
		 video<<vidimg;
		 //cv::imwrite(filename,vidimg);
	 }
	 video.release();
 }
#include "Process.h"

Process::Process(float **data,int nf,int w,int h){
	num_of_frames=nf;
	width=w;
	height=h;
	maximum=0;
	minimum=DBL_MAX;
	for(int i=0;i<nf;i++){
		cv::Mat m(height,width,CV_32FC1,data[i]);
		checkmax(m);
		images.push_back(m);
	}
	visualizer = new Visualizer(data,nf,w,h);
}

Process::~Process(){

}

void Process::checkmax(Mat m){
	double minval,maxval;
	minMaxLoc(m,&minval,&maxval);
	maximum=(maximum<maxval?maxval:maximum);
	minimum=(minimum>minval?minval:minimum);
}

void Process::make_contours(){
	vector<vector<cv::Point> > contours;
	for(int i=0;i<num_of_frames;i++){
		cv::Mat m;
		images[i].convertTo(m,CV_8UC1,255);
		clean_image(m);
		Mat mm;m.copyTo(mm);
		cv::findContours(mm,contours,CV_RETR_EXTERNAL,CV_CHAIN_APPROX_NONE);
		clean_noise_contours(contours);
		//cout<<contours.size()<<endl;
		//imshow("showm",m);
		//waitKey();
		listofcontours.push_back(contours);
		mm.release();
	}
	visualizer->add_contours(listofcontours);
	hype = new Hypothesis(listofcontours,maximum,images);
	hype->track();
	visualizer->add_minmax(minimum,maximum);
	visualizer->add_nocolors(hype->get_nocolors());
	visualizer->add_fcc(hype->get_fcc());
	//visualizer->add_joinvec(hype->get_joinvec());
	visualizer->add_conditions(hype->get_conditions());
	visualizer->add_allpoints(hype->get_allpoints());
	visualizer->add_validity(hype->get_validity());
	visualizer->add_labels(hype->get_labels());
	visualizer->add_links(hype->get_framelinks());
	visualizer->view();
}

void Process::clean_image(Mat &img){
	Mat strel=cv::getStructuringElement(cv::MORPH_ELLIPSE,cv::Size(MORPH_SIZE,MORPH_SIZE));
	cv::erode(img,img,strel,cv::Point(-1,-1),3);
	cv::dilate(img,img,strel,cv::Point(-1,-1),3);
}

void Process::clean_noise_contours(vector<vector<cv::Point> >& contours){
	for(int i=0;i<contours.size();i++){
		double csize=contourArea(contours[i]);
		if(csize < width*height*MINCSIZE){
			contours.erase(contours.begin()+i);
			i--;
		}
	}
}
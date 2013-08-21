#include "rawReader.h"


rawReader::rawReader(void)
{
}


rawReader::~rawReader(void)
{
}

void rawReader::readfile(string dir,int start,int end,int w,int h,bool flip){
	int numfiles=end-start+1;
	width=w;height=h;
	data = new float*[numfiles];
	if(!flip){
		for(int i=start,j=0;i<=end;i++,j++){
			data[j]= new float[width*height];
			char num[10];
			sprintf(num,"%03d",i);
			string fname=dir+"/"+string(num)+".raw";
			FILE *fp;
			fp=fopen(fname.c_str(),"r");
			fread(data[j],sizeof(float),width*height,fp);
			fclose(fp);
	/*		
			Test to see of the data was any good checked out fine
			cv::Mat m(height,width,CV_32FC1,data[j]);
			cv::imshow("showme",m);
			cv::waitKey(100);
	*/
		}
	}
	else{
		for(int i=end,j=0;i>=start;i--,j++){
			data[j]= new float[width*height];
			char num[10];
			sprintf(num,"%03d",i);
			string fname=dir+"/"+string(num)+".raw";
			FILE *fp;
			fp=fopen(fname.c_str(),"r");
			fread(data[j],sizeof(float),width*height,fp);
			fclose(fp);
	/*		
			Test to see of the data was any good checked out fine
			cv::Mat m(height,width,CV_32FC1,data[j]);
			cv::imshow("showme",m);
			cv::waitKey(100);
	*/
		}
	}
}

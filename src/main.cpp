#include "rawReader.h"
#include "Visualizer.h"
#include "Process.h"

#define RAWWIDTH 576
#define RAWHEIGHT 576
#define DATAPATH "E:/Dropbox/cotton_814/3break_2000_9fringes"
#define START_IMAGE 1
#define END_IMAGE 26
#define NO_IMAGES END_IMAGE-START_IMAGE+1

int main(){
//	osg::Node* blah= osgDB::readNodeFile("cow.osg");
	rawReader reader;
	reader.readfile(DATAPATH,START_IMAGE,END_IMAGE,RAWWIDTH,RAWHEIGHT);
	Process p(reader.data,NO_IMAGES,RAWWIDTH,RAWHEIGHT);
	p.make_contours();
	//Visualizer v(reader.data,NO_IMAGES,RAWWIDTH,RAWHEIGHT);
	return 0;
}
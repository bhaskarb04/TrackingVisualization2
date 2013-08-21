#pragma once
#include <string>
#include <stdio.h>
#include <iostream>
#include <opencv\cv.h>
#include <opencv\highgui.h>

using namespace cv;
using namespace std;

class rawReader
{
	int width;
	int height;
public:
	rawReader(void);
	~rawReader(void);
//Read the raw file
	void readfile(string,int,int,int,int,bool flip=false);
//Data variable used to store all the raw values
	float **data;
};


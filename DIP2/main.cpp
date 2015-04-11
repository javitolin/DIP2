/*
 * main.cpp
 *
 *  Created on: Apr 1, 2015
 *      Author: jdorfsman
 */

#include "highgui/highgui.hpp"
#include "core/core.hpp"
#include "imgproc/imgproc.hpp"
#include "opencv.hpp"
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

using namespace cv;
using namespace std;

#define INTENSITY 255
Mat src, dst;
int outputImageExists = 0;
char* targetHistogramFile;
char* targetImageFile;
double receivedHistogram[256];
float sk[256];
double Gr[256];
double Gb[256];
double Gg[256];
double histoMatchesR[256];
double histoMatchesG[256];
double histoMatchesB[256];
double rHistogram[256];
double bHistogram[256];
double gHistogram[256];
vector<Mat> bgr;

void createDstImage(){
	dst = Mat::zeros(src.rows,src.cols,CV_8UC3);
	for(int i = 0; i < dst.rows; i++){
		for(int j = 0; j < dst.cols; j++){
			dst.at<Vec3b>(i,j)(0) = bgr[0].at<uchar>(i,j);
			dst.at<Vec3b>(i,j)(2) = bgr[1].at<uchar>(i,j);
			dst.at<Vec3b>(i,j)(1) = bgr[2].at<uchar>(i,j);
		}
	}
}
void drawImages(Mat img, double toUse[]){
	for(int i = 0; i < img.rows; i++){
		for(int j = 0; j < img.cols; j++){
			img.at<uchar>(i,j) = toUse[img.at<uchar>(i,j)];
		}
	}
}
void calculateValues(){
	double sumForNowSK = 0;
	double sumForNowGr = 0;
	double sumForNowGb = 0;
	double sumForNowGg = 0;
	for(int i = 0; i < 256; i++){
		sk[i] = (sumForNowSK + receivedHistogram[i])/(INTENSITY-1);
		sumForNowSK = sk[i];

		Gr[i] = sumForNowGr + (INTENSITY-1) * rHistogram[i];
		Gr[i] /= (src.rows*src.cols);
		sumForNowGr = Gr[i];

		Gg[i] = sumForNowGg + (INTENSITY-1) * gHistogram[i];
		Gg[i] /= (src.rows*src.cols);
		sumForNowGg = Gg[i];

		Gb[i] = sumForNowGb + (INTENSITY-1) * bHistogram[i];
		Gb[i] /= (src.rows*src.cols);
		sumForNowGb = Gb[i];
	}
	for(int i = 0; i < 256; i++){
		sk[i] = sk[i] * INTENSITY;
		sk[i] = round(sk[i]);
		Gr[i] = round(Gr[i]);
		Gg[i] = round(Gg[i]);
		Gb[i] = round(Gb[i]);
	}
	int countMatches = 0;
	for(int i = 0; i < 256; i++){
		for(int j = 0; j < 256; j++){
			if(sk[i] == Gr[j]){
				histoMatchesR[j] += sk[i];
				countMatches++;
			}
		}
	}
	cout << "matches: " << countMatches << endl;
	countMatches = 0;
	for(int i = 0; i < 256; i++){
		for(int j = 0; j < 256; j++){
			if(sk[i] == Gg[j]){
				histoMatchesG[j] += sk[i];
				countMatches++;
			}
		}
	}
	cout << "matches: " << countMatches << endl;
	countMatches = 0;
	for(int i = 0; i < 256; i++){
		for(int j = 0; j < 256; j++){
			if(sk[i] == Gb[j]){
				histoMatchesB[j] += sk[i];
				countMatches++;
			}
		}
	}
	cout << "matches: " << countMatches << endl;
}
void initializeHistograms(){
	for(int i = 0; i < 256; i++){
		rHistogram[i] = 0;
		bHistogram[i] = 0;
		gHistogram[i] = 0;
		histoMatchesR[i] = 0;
		histoMatchesG[i] = 0;
		histoMatchesB[i] = 0;
		Gg[i] = 0;
		Gr[i] = 0;
		Gb[i] = 0;
		sk[i] = 0;
	}
}
void createHistogram(Mat img, double hist[]){
	for(int i = 0; i < img.rows; i++){
		for(int j = 0; j < img.cols; j++){
			hist[img.at<uchar>(i, j)]++;
		}
	}
}

void readHistogram(){
	ifstream myfile;
	string line;
	myfile.open (targetHistogramFile);
	int lineNum = 0;
	if (myfile.is_open())
	  {
	    while ( getline (myfile,line) )
	    {
	      receivedHistogram[lineNum++] = atoi(line.c_str());
	    }
	    myfile.close();
	  }
}
void printHistogram(){
	for(int i = 0; i < 256; i++){
		cout << "Line: " << i << " Intensity: " << receivedHistogram[i] << endl;
	}
}

int main(int argc, char * argv[]){
	char* inputImageFile = 0;
	if(argc < 3){
		cout << "Usage is: MatchHistogram <input_image_file> <target_histogram> (output_image_file)?" << endl;
		return -1;
	}
	else if(argc == 3){
		inputImageFile = argv[1];
		targetHistogramFile = argv[2];
	}
	else if(argc == 4){
		inputImageFile = argv[1];
		targetHistogramFile = argv[2];
		targetImageFile = argv[3];
		outputImageExists = 1;
	}
	src = imread(inputImageFile,1);
	readHistogram();
	split(src, bgr);
	initializeHistograms();
	createHistogram(bgr[0],bHistogram);
	createHistogram(bgr[1],gHistogram);
	createHistogram(bgr[2],rHistogram);
	calculateValues();
	drawImages(bgr[0],histoMatchesB);
	drawImages(bgr[1],histoMatchesG);
	drawImages(bgr[2],histoMatchesR);
	createDstImage();
	namedWindow("Source Image", WINDOW_AUTOSIZE);  // Create a window for display.
	imshow("Source Image", src);  // Show Before Image
	namedWindow("Dst Image", WINDOW_AUTOSIZE);  // Create a window for display.
	imshow("Dst Image", dst);  // Show Before Image
	if(outputImageExists == 1){
		imwrite(targetImageFile,dst);
	}
	waitKey();
	return 0;
}


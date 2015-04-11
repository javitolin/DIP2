/*
 * main.cpp
 *
 *  Created on: Apr 1, 2015
 *      Author: jdorfsman
 */


//////////////////// IMAGE SIZE: 253 * 62
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

#define INTENSITY 256
#define HISTMATCH_EPSILON 1.5
Mat src, dst;
int outputImageExists = 0;
char* targetHistogramFile;
char* targetImageFile;
double receivedHistogram[256];
double sk[256];
double Gz[256];
double epsilon = 1.5;
double histoMatchesR[256];
double imageHistogram[256];

void drawImages() {
	dst = Mat::zeros(src.rows, src.cols, src.type());
	for (int i = 0; i < src.rows; i++) {
		for (int j = 0; j < src.cols; j++) {
			dst.at < uchar > (i, j) = histoMatchesR[src.at < uchar > (i, j)];
		}
	}
}
void calculateValues() {
	double sumForNowSK = 0;
	double sumForNowGz = 0;
	for (int i = 0; i < INTENSITY; i++) {
		sk[i] = (INTENSITY - 1) * imageHistogram[i] + sumForNowSK;
		Gz[i] = (INTENSITY - 1) * receivedHistogram[i] + sumForNowGz;
		sumForNowSK = sk[i];
		sumForNowGz = Gz[i];
	}
	for (int i = 0; i < INTENSITY; i++) {
		sk[i] = sk[i] / (src.rows * src.cols);
		Gz[i] = Gz[i] / (src.rows * src.cols);
		sk[i] =
				(round(sk[i]) < (INTENSITY - 1) ? round(sk[i]) : (INTENSITY - 1));
		Gz[i] =
				(round(Gz[i]) < (INTENSITY - 1) ? round(Gz[i]) : (INTENSITY - 1));
		cout << "Gz: " << Gz[i] << " SK: " << sk[i] << endl;
	}

	int countMatches = 0;
	for (int i = 0; i < INTENSITY; i++) {
		for (int j = 0; j < INTENSITY; j++) {
			if (abs(sk[i] - Gz[j]) < epsilon) {
				if (histoMatchesR[j] == 0) {
					histoMatchesR[j] = sk[i];
				}
				countMatches++;
			}
		}
	}
	cout << "Matches: " << countMatches << endl;
}
/*
 void calculateValues(){
 double sumForNowSK = 0;
 double sumForNowGr = 0;
 for(int i = 0; i < 256; i++){
 sk[i] = (sumForNowSK + receivedHistogram[i])/(INTENSITY-1);
 Gz[i] = sumForNowGr + ((INTENSITY-1) * imageHistogram[i])/(src.rows*src.cols);
 sumForNowSK = sk[i];
 sumForNowGr = Gz[i];
 }
 for(int i = 0; i < 256; i++){
 sk[i] = round(sk[i]);
 Gz[i] = round(Gz[i]);
 cout << "GR: " << Gz[i] << " SK: " << sk[i] << endl;
 }
 int countMatches = 0;
 for(int i = 0; i < 256; i++){
 for(int j = 0; j < 256; j++){
 if(sk[i] == Gz[j]){
 histoMatchesR[j] += sk[i];
 countMatches++;
 }
 }
 }
 cout << "Matches: " << countMatches << endl;
 }*/
void initializeHistograms() {
	for (int i = 0; i < 256; i++) {
		imageHistogram[i] = 0;
		histoMatchesR[i] = 0;
		Gz[i] = 0;
		sk[i] = 0;
		receivedHistogram[i] = 0;
	}
}
void createHistogramFromImage() {
	for (int i = 0; i < src.rows; i++) {
		for (int j = 0; j < src.cols; j++) {
			imageHistogram[src.at < uchar > (i, j)]++;
		}
	}
}

void readHistogramFromFile() {
	ifstream myfile;
	string line;
	myfile.open(targetHistogramFile);
	int lineNum = 0;
	if (myfile.is_open()) {
		while (getline(myfile, line)) {
			receivedHistogram[lineNum++] = atoi(line.c_str());
		}
		myfile.close();
	}
}
void printHistogram() {
	for (int i = 0; i < 256; i++) {
		cout << "Line: " << i << " Intensity: " << receivedHistogram[i] << endl;
	}
}

int main(int argc, char * argv[]) {
	char* inputImageFile;
	if (argc < 3) {
		cout
				<< "Usage is: MatchHistogram <input_image_file> <target_histogram> (output_image_file)?"
				<< endl;
	} else if (argc == 3) {
		inputImageFile = argv[1];
		targetHistogramFile = argv[2];
	} else if (argc == 4) {
		inputImageFile = argv[1];
		targetHistogramFile = argv[2];
		targetImageFile = argv[3];
		outputImageExists = 1;
	}
	src = imread(inputImageFile, 0);
	initializeHistograms();
	createHistogramFromImage();
	//Calculating cumulative histogram of src
	double total = src.rows*src.cols;
	double probSrc[255];
	int newValuesSrc[255];
	double cuml = 0;
	for(int j = 0; j < 256; j++)
	{
		probSrc[j] = imageHistogram[j]/total; // Probability of each value in image
	    cuml = cuml + probSrc[j]; // Cumulative probability of current and all previous values
	    double cdfmax = cuml * 255;   // Cumulative probability * max value
	    newValuesSrc[j] = (int) round(cdfmax);
	    cout << imageHistogram[j] << " "<< probSrc[j] << " " << newValuesSrc[j] << endl;
	}
	readHistogramFromFile();
	cout << "########################" << endl;
	//Calculating cumulative histogram from file
	double probDst[255];
	int newValuesDst[255];
	int maxNewValue = 0;
	int sumOfHistogram = 0;
	cuml = 0;
	for(int j = 0; j < 256; j++)
	{
		probDst[j] = receivedHistogram[j]/total; // Probability of each value in image
	    cuml = cuml + probDst[j]; // Cumulative probability of current and all previous values
	    double cdfmax = cuml * 255;   // Cumulative probability * max value
	    newValuesDst[j] = (int) round(cdfmax);
	    sumOfHistogram+=receivedHistogram[j];
	    cout << receivedHistogram[j] << " "<< probDst[j] << " " << newValuesDst[j] << endl;
	}
	cout << sumOfHistogram << endl;
	for (int i = 0; i < 256; i++) {
		for (int j = 0; j < 256; j++) {
			if (newValuesDst[i] == newValuesSrc[j]) {
				histoMatchesR[j] += newValuesSrc[i];
			}
		}
	}
	drawImages();


	//calculateValues();

	namedWindow("Source Image", WINDOW_AUTOSIZE); // Create a window for display.
	imshow("Source Image", src);  // Show Before Image
	namedWindow("Dst Image", WINDOW_AUTOSIZE);  // Create a window for display.
	imshow("Dst Image", dst);  // Show Before Image
	if (outputImageExists == 1) {
		imwrite(targetImageFile, dst);
	}
	waitKey();
	return 0;
}

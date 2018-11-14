// ConsoleApplication1.cpp: 定義主控台應用程式的進入點。
//
//#include "stdafx.h"
#include <iostream>
#include <cstdlib>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <cmath>

using namespace std;
using namespace cv;
//using namespace std::piecewise_construct;

//void 	bilinear_interpolation(Mat& input, Mat& output, float scalingFactor);

int main(int argc, char** argv) {
	int a[256] = { 0 };
	float s[256] = { 0 };
	float sum = 0;
	//cout << "0";
	Mat input = imread("mj.tif", 0);
	//cout << "1";
	Mat output = Mat(input.rows, input.cols, input.type());
	for (int i = 0; i < input.rows; i++) {
		for (int j = 0; j < input.cols; j++) {
			a[input.at<uchar>(i, j)]++;
		}
	}
	
	
	//cout <<"2";
	for (int i = 0; i < 256; i++) {
		sum += a[i];
		s[i] = sum;
	}
	//cout << "3";
	for (int i = 0; i < 256; i++) {
		s[i] = s[i] / sum * 255;
	}
	//cout << "4";
	for (int i = 0; i < input.rows; i++) {
		for (int j = 0; j < input.cols; j++) {
			//output.at<uchar>(i, j) = s[input.at<uchar>(i, j)]++;
			output.at<uchar>(i, j) = s[input.at<uchar>(i, j)];
		}
	}
	//cout << "5";
	imshow("yeah", output);
	//imshow("Opencv build-in function", outputImg2);
	waitKey(0);

	imwrite("output.tif", output);

	return 0;
}



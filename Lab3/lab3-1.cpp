// i am lab3-1
//#include "stdafx.h"
#include <iostream>
#include <cstdlib>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <cmath>

using namespace std;
using namespace cv;
void otsu(Mat& output);
int main(int argc, char** argv) {

	Mat input = imread("input.jpg", 0);
	Mat output = input.clone();
	//計算直方圖
	otsu(output);
	
	imshow("yeah", output);
	imwrite("output.jpg", output);
	waitKey(0);
	return 0;
}
void otsu(Mat& output) {
	int a[256] = {0};
	int th;
	double meanb, meano, sb, so, var, varmax=0;
	//計算直方圖
	for (int i = 0; i < output.rows; i++) {
		for (int j = 0; j < output.cols; j++) {
			a[output.at<uchar>(i, j)]++;
		}
	}
	for (int i = 1; i < 255; i++) {
		meanb = 0; meano = 0; sb = 0; so = 0;
		for (int j = 0; j < 256; j++) {
			if (j <= i) {
				meanb += (a[j] * j);
			}
			else if (j > i) {
				meano += (a[j] * j);
			}

			for (int j = 0; j <= i; j++) {
				sb += a[j];			//算前面的總數
			}
			float mb = meanb / sb;			 //算前面的mean
			
			for (int j = i + 1; j < 256; j++) {
				so += a[j];          //算後面的總數
			}
			float mo = meano / so;			//算後面的mean
			
			var = sb * so*(mo - mb)*(mo - mb);
			if (var > varmax) {
				th = i;
				varmax = var;
				//cout << th<<" "<<mo<<" "<<mb;
			}
		}
	}
	//while(1){}
	for (int i = 0; i < output.rows; i++) {
		for (int j = 0; j < output.cols; j++) {
			if (output.at<uchar>(i, j) > th) {
				output.at<uchar>(i, j) = 255;
			}
			else {
				output.at<uchar>(i, j) = 0;
			}
		}
	}
}

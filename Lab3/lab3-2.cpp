// i am lab3-2
//#include "stdafx.h"
#include <iostream>
#include <cstdlib>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <cmath>
#include <vector>

using namespace std;
using namespace cv;


int main(int argc, char** argv) {
	
	
	//Mat output = Mat(input.rows, input.cols, input.type());
	Mat input = imread("output(ostu).jpg", 0);    
	Mat output = input.clone();
	int label = 1;
	int labelup = 0;
	for (int i = 1; i < output.rows-1; i++) {
		for (int j = 1; j < output.cols-1; j++) {
			if (output.at<uchar>(i, j) != 0) {//not black
				if (output.at<uchar>(i, j-1)!=0&& output.at<uchar>(i-1, j)!=0 && (i-1)>=0&&(j-1)>=0) {//left&up have labeled
					output.at<uchar>(i, j) = output.at<uchar>(i, j - 1);//left
					labelup = output.at<uchar>(i - 1, j);
					//把以前的改掉
					for (int k = 1; k < output.rows-1; k++) { 
						for (int m = 1; m < output.cols-1; m++) {
							if (output.at<uchar>(k, m) == labelup) {//if equals up then change to left
								output.at<uchar>(k, m) = label;
							}
						}
					}
				}
				else if (output.at<uchar>(i, j - 1) != 0 && (j-1)>=0) {//just left 
					output.at<uchar>(i, j) = output.at<uchar>(i, j - 1);//left
				}
				else if (output.at<uchar>(i-1, j) != 0 && (i-1)>=0) {//just up
					output.at<uchar>(i, j) = output.at<uchar>(i-1, j);//up
				}
				else {//none have labeled
					output.at<uchar>(i, j) = label++;
					cout << label << " ";
				}
			}
		}
	}
	cvtColor(output, output, CV_GRAY2RGB);
	RNG rng(99999);
	vector<Vec3b> color(label-1);//剛剛++了
	for (int i = 1; i < color.size(); i++) {
		color[i] = Vec3b(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255));
	}
	for (int i = 1; i < output.rows-1; i++) {
		for (int j = 1; j < output.cols-1; j++) {
			if (output.at<uchar>(i, j) != 0) {
				output.at<Vec3b>(i, j) = color[output.at<uchar>(i, j)];
			}
		}
	}

	imshow("show", output);
	waitKey(0);
	imwrite("output.jpg", output);
	return 0;
}
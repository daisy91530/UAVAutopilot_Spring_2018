// i am lab2-2
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
	//int mask[3][3] = { 0, 1, 0, 1, -4, 1, 0, 1, 0 };
	//cout << "0";
	Mat input = imread("mj.tif", 0);  //read signed 等改
	//cout << "1";
	Mat output = Mat(input.rows, input.cols, input.type());
	for (int i = 0; i < input.rows; i++) {
		for (int j = 0; j < input.cols; j++) {
			if (i == 0 || j == 0 || i == input.rows-1 || j == input.cols-1) {
				output.at<uchar>(i, j) = input.at<uchar>(i, j);
			}
			else {
				int uns = input.at<uchar>(i - 1, j - 1) * 0 + input.at<uchar>(i - 1, j) * 1 + input.at<uchar>(i - 1, j + 1) * 0
				+ input.at<uchar>(i, j - 1) * 1 + input.at<uchar>(i, j) * (-4) + input.at<uchar>(i, j + 1) * 1
				+ input.at<uchar>(i + 1, j - 1) * 0 + input.at<uchar>(i + 1, j) * 1 + input.at<uchar>(i + 1, j + 1) * 0;
				if (uns < 0) output.at<uchar>(i, j) = 0;
				else if (uns > 255) output.at<uchar>(i, j) = 255;
				else output.at<uchar>(i, j) = uns;
			}
		}
	}

	imshow("yeah", output);
	//imshow("Opencv build-in function", outputImg2);
	waitKey(0);
	imwrite("output.tif", output);

	return 0;
}
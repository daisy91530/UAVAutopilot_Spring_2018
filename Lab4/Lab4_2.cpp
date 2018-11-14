#include <cstdio>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>

using namespace std;
using namespace cv;

// write down your warping function here
Mat warp(InputArray src, InputArray M, Size dsize)
{
	int x, y, x_, y_;
	Mat matrix = M.getMat();
	Mat input = src.getMat();
	Mat output;

	for (x = 0; x < dsize.width; x++)
	{
		for (y = 0; y < dsize.height; y++)
		{
			x_ = (matrix.at<uchar>(0, 0) * x + matrix.at<uchar>(0, 1) * y + matrix.at<uchar>(0, 2)) / (matrix.at<uchar>(2, 0) * x + matrix.at<uchar>(2, 1) * y + matrix.at<uchar>(2, 2));
			y_ = (matrix.at<uchar>(1, 0) * x + matrix.at<uchar>(1, 1) * y + matrix.at<uchar>(1, 2)) / (matrix.at<uchar>(2, 0) * x + matrix.at<uchar>(2, 1) * y + matrix.at<uchar>(2, 2));

			output.at<uchar>(x_, y_) = input.at<uchar>(x, y);
		}
	}

	return output;
	 
			
}




void onMouse(int event, int x, int y, int flags, void* param) {
	vector<Point2f>* ptr = (vector<Point2f>*) param;
	if (event == CV_EVENT_LBUTTONDOWN) {
		ptr->push_back(Point2f(x, y));
	}
}

int main() {
	VideoCapture cap(1);
	if (!cap.isOpened()) {
		return -1;
	}
	Mat image;
	image = imread("Lab4.jpg");

	Mat frame;
	cap >> frame;
	cout << frame.size() << endl;
	//vector<Point2f> cap_corner;
	//vector<Point2f> img_corner;

	Point2f cap_corner[4];
	Point2f img_corner[4];

	cap_corner[0] = Point2f(0, 0);
	cap_corner[1] = Point2f(frame.cols - 1, 0);
	cap_corner[2] = Point2f(frame.cols - 1, frame.rows - 1);
	cap_corner[3] = Point2f(0, frame.rows - 1);

	
	img_corner[0] = Point2f(image.cols*0.2, image.rows*0.3);
	img_corner[1] = Point2f(image.cols*0.5, image.rows*0.2);
	img_corner[2] = Point2f(image.cols*0.8, image.rows*0.8);
	img_corner[3] = Point2f(image.cols*0.25, image.rows*0.9);
	
	// add the corner of frame into cap_corner	

	namedWindow("img", CV_WINDOW_AUTOSIZE);
	//setMouseCallback("img", onMouse, &img_corner);
	/*
	while (img_corner.size()<4) {
		imshow("img", image);
		if (waitKey(1) == 27) break;
	
	*/
	
	Mat img_out = image.clone();
	Mat img_temp = image.clone();
	Mat h = getPerspectiveTransform(cap_corner, img_corner);
	img_temp = warp(frame, h, img_temp.size());// 繪製坐標變換[之前]與[之後]的示意圖
	//warpAffine(image, frame, h, frame.size());// call your warping function
	imshow("warped", img_temp);
	
	
	Point poly[4];
	for (int i = 0; i < 4; i++) {
		poly[i] = img_corner[i];
	}
	
	while (1) {
		cap >> frame;
		//warpAffine(image, frame, h, frame.size());// call your warping function 	
		warpPerspective(frame, img_temp, h, img_temp.size());
		fillConvexPoly(img_out, poly, 4, Scalar(0), CV_AA);
		img_out = img_out + img_temp;
		imshow("img", img_out);
		if (waitKey(1) == 27) break;
	}
	return 0;
}


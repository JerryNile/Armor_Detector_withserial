#include<opencv2/opencv.hpp>
#include<opencv2/imgproc.hpp>
#include<iostream>
#include<vector>
#include<math.h>
#include<string>

using namespace cv; 
using namespace std;
using namespace cv::ml;

int  main(int argc,char ** argv){
    VideoCapture cap("./IMG_2223.mp4");
	if (!cap.isOpened()) {
		cerr << " can not open a camera or file" << endl;
		return -1;
	}


	for (;;) {
		Mat temp;
		//从cap中读出一帧  存放在frame
		cap >> temp;

		if (frame.empty()) {
			break;
		}
		//将读到的图片转化成灰度图
        Mat grayImg;
        Mat binBrightImg;
		cvtColor(temp, grayImg, CV_BGR2GRAY,1);
		threshold(grayImg,binBrightImg,)
	}
	return 0;
}
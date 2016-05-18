#ifndef SPEEDRANGE_H_
#define SPEEDRANGE_H_

#include "opencv2/video/tracking.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <iostream>

using namespace cv;
using namespace std;

enum speedDegree{ duruyor, yavas, hizli, kellekoltukta };

class SpeedRange {
public:
	SpeedRange();
	int findRange(Mat frameIn);
private:
	int getSpeed(const Mat& flow, Mat& cflowmap, int step,
		double, const Scalar& color);
	Mat flow;
	Mat cflow;
	Mat frame;
	Mat frameorg;
	Mat gray;
	Mat prevgray;
	Mat uflow;
	int speed;
	int count;
	Size size;//the dst image size,e.g.100x100
	Rect rectCrop;
	Mat dMat;
	int range;
};

#endif /* SPEEDRANGE_H_ */
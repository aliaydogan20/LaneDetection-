#include "SpeedRange.h"

SpeedRange::SpeedRange()
{
	speed = 0;
	count = 0;
	size = Size(100, 100);
	rectCrop = Rect(0, 0, 100, 45);
	range = -1;
}

int SpeedRange::getSpeed(const Mat& flow, Mat& cflowmap, int step,
	double, const Scalar& color)
{
	double num = 0;
	for (int y = 0; y < cflowmap.rows; y += step)
		for (int x = 0; x < cflowmap.cols; x += step)
		{
			const Point2f& fxy = flow.at<Point2f>(y, x);
			num += fxy.y;
		}
	return int(fabs(num) * 1000);
}

int SpeedRange::findRange(Mat frameIn)
{
	frame = frameIn;
	resize(frame, frame, size);//resize image
	frame = Mat(frame, rectCrop);
	cvtColor(frame, gray, COLOR_BGR2GRAY);

	if (!prevgray.empty())
	{
		calcOpticalFlowFarneback(prevgray, gray, uflow, 0.5, 3, 15, 3, 5, 1.2, 0);
		cvtColor(prevgray, cflow, COLOR_GRAY2BGR);
		uflow.copyTo(flow);
		speed += getSpeed(flow, cflow, 16, 1.5, Scalar(0, 255, 0));
		++count;

		if (count == 20){
			speed = speed / 20;
			if (speed < 600)
				range = duruyor;
			else if (speed >= 1000 && speed < 4000)
				range = yavas;
			else if (speed >= 6000 && speed < 10000)
				range = hizli;
			else if (speed >= 14000)
				range = kellekoltukta;
			count = 0;
			speed = 0;
		}
	}
	std::swap(prevgray, gray);
	return range;
}

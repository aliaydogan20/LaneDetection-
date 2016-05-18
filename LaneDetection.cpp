#include <stdio.h>
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/opencv.hpp>
#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <opencv/cvaux.h>
#include <iostream>
#include <opencv/cxcore.h>
#include <vector>
#include <math.h>
#include <string>
#include <cstdlib>
#include <time.h>
#include <unistd.h>
#include <wiringPi.h>
#include <pthread.h>
#include "SpeedRange.h"

#define PI 3.1415926
string getCarColor(int startX, int startY, int width, int height, IplImage* inp);
double imageMean(IplImage * inp);
double imageStd(IplImage * inp, double mean);
void detect(IplImage *img);

using namespace cv;
using namespace std;

CvMemStorage* haarStorage = cvCreateMemStorage(0);
CvMemStorage* storage = cvCreateMemStorage(0);
CvHaarClassifierCascade* cascade = ( CvHaarClassifierCascade* )cvLoad( "/home/pi//Desktop/Speed Detection/group1cars.xml", 0, 0, 0 );
int flag =0;
void *tempL;
void *blink(void * arg);	

int main(void)
{
	CvCapture *cap=cvCaptureFromCAM(0);
	IplImage* dstCanny;
	IplImage * gray;
	IplImage *grey;
	IplImage *smallImg;
	IplImage* halfFrame;
	int error;
	pthread_t tid;
	wiringPiSetup () ;
	
  	pinMode (0, OUTPUT) ;
	
	if(error = pthread_create(&tid, NULL, blink, tempL)){
		cout << "Thread cannot create" << endl;
	}

	if (cap == NULL) {
		fprintf(stderr, "Error: Can't open video\n");
		return -1;
	}

	SpeedRange rangeFinder = SpeedRange();
	CvHaarClassifierCascade* cascade = ( CvHaarClassifierCascade* )cvLoad( "/home/pi/Desktop/Speed Detection/group1cars.xml", 0, 0, 0 );
	assert( cascade && storage && cap);

	 const double Scale = 2.0;
     const double ScaleFactor = 1.05;
     const int MinNeighbours = 2;

	 CvSeq* lines=0;
	 CvSeq* cars;
	 int i=0;
	 CvFont font;
	 cvInitFont(&font, CV_FONT_VECTOR0, 0.25f, 0.25f);
	 IplImage* src=cvQueryFrame(cap);

	 while(1)
	 {
			int prevSpeed = -2;   // Onceki hiz
		    int speed = -1; 			// Simdiki hiz
		    src= cvQueryFrame(cap);  // Cameradan frame alma
	       Mat newImage=cvarrToMat(src); // Mat tipine cevirme
		   string speedDisplay="";					// Hiz yazdirmak icin tanimlandi.
		
			// 10 frame'de islem yapiliyor.
			if((i%10)!=0)
	       {
	        	cvResetImageROI(src);
	      		 i++;
	     		  continue;
			}
			i++;
				
				
			halfFrame = cvCreateImage(cvSize(src->width / 2, src->height / 2), IPL_DEPTH_8U, 3);
		    cvPyrDown(src, halfFrame,CV_GAUSSIAN_5x5);
		
			 dstCanny=cvCreateImage(cvGetSize(src),IPL_DEPTH_8U,1);
			 gray=cvCreateImage(cvSize(src->width, src->height), IPL_DEPTH_8U, 1);
			 smallImg = cvCreateImage(cvSize(cvRound(src->width / Scale), cvRound(src->height / Scale)), IPL_DEPTH_8U, 1);
	 	     grey = cvCreateImage(cvGetSize(src), IPL_DEPTH_8U, 1);
		
	
		speed = rangeFinder.findRange(newImage);
	
		
		if (speed != prevSpeed)
		{
			if(speed==duruyor)
			{
				speedDisplay="DURUYOR";
			}
			if(speed==yavas)
			{
		
				speedDisplay="YAVAS GIDIYOR";
			}
			if(speed>prevSpeed)
			{
				speedDisplay="HIZLI GIDIYOR";
			}
			if(speed==kellekoltukta)
			{

				speedDisplay="KELLE KOLTUKTA";	
			}
		}
		
		prevSpeed = speed;
		
		putText(newImage,speedDisplay, cvPoint(130, 40), FONT_HERSHEY_PLAIN, 3, Scalar(30, 20, 100), 2, CV_AA);
		
	 	cvClearMemStorage(storage);
 
		// Arac bulma  ve aracin rengini bulma 
    		detect(src);
		
		
		cvResetImageROI(src);
	 	
	 
	 	 int halfWidth = src->width / 2;
         int halfHeight = src->height / 2;
         int startX = halfWidth - (halfWidth / 2);	

		 cvCvtColor(src, grey, CV_RGB2GRAY);
		 cvResize(grey, smallImg, 1);
		 cvEqualizeHist(smallImg, smallImg);
		
       	CvRect roi_rect = cvRect(120, 240,src->width - 120, src->height);
		 cvSetImageROI((src), (roi_rect));
		 cvSetImageROI((gray), (roi_rect));
		 cvSetImageROI((dstCanny), (roi_rect));
	
			
		 IplImage* croppedImg = cvCreateImage(cvGetSize(src), IPL_DEPTH_8U, 3);
		 cvCopy(src, croppedImg,NULL);



		int i = 15;
		for (int j = 0; j < croppedImg->width; j++)
		{
				CvScalar s;
				s = cvGet2D(croppedImg, i, j); // get the (i,j) pixel value
				
				// Bu 3 degere bakilarak titresim icin flag ayarlanir.
				if (s.val[0] <=20 ||
					s.val[1] <= 22 ||
					s.val[2] <= 24)
				{
				// flag 1 ise titrestir.
					flag = 1;		
				}
				else
					flag = 0;
				
		}

		cvCvtColor(croppedImg, gray, CV_RGB2GRAY);
		cvSmooth(gray, gray, CV_GAUSSIAN, 5, 5);

		double meanVal;
		double stdev; 
		meanVal=imageMean(gray);
		stdev=imageStd(gray,meanVal);

		// Threshold islemi
		cvThreshold(gray, gray, meanVal + 1.6 * stdev, 255, CV_THRESH_BINARY);
		cvCanny(gray, dstCanny, 20, 100, 3);

		lines = cvHoughLines2(dstCanny, storage, CV_HOUGH_STANDARD, 1, CV_PI / 180, 50, 50, 100);
	
		CvPoint prePt1,prePt2;

		prePt1.x=cvRound(0);
		prePt2.x=cvRound(0);
		prePt1.y=cvRound(0);
        prePt2.y=cvRound(0);
		int deg=0;


		for (int i = 0; i < lines->total; i++)
		{
			
			float *line = (float*)cvGetSeqElem(lines, i);
			float rho = line[0];
			float theta = line[1];
			CvPoint pt1, pt2;
			double a = cos(theta), b = sin(theta);
			double x0 = a*rho, y0 = b*rho;
			pt1.x = cvRound(x0 + 1000 * (-b));
			pt1.y = cvRound(y0 + 1000 * (a));
			pt2.x = cvRound(x0 - 1000 * (-b));
			pt2.y = cvRound(y0 - 1000 * (a));
			
			// Cizgi sayisini azaltma	
			if(((prePt1.x-pt1.x>325)||(pt1.x-prePt1.x>325) ) && ((prePt1.y-pt1.y>325)||(pt1.y-prePt1.y>325))
			   &&((prePt2.x-pt1.x>325)||(pt2.x-prePt1.x>325) ) && ((prePt2.y-pt2.y>325)||(pt2.y-prePt2.y>325)))
			{
				deg=1;
			}
			
			prePt1.x=pt1.x;
			prePt1.y=pt1.y;
			prePt2.x=pt2.x;
			prePt2.y=pt2.y;
			
			int dx = pt2.x - pt1.x;
			int dy = pt2.y - pt1.y;
			double angle = atan2(dy, dx) * 180 / PI;
			
			if (abs(angle) <= 10)
				continue;
				
			if (pt1.y > pt2.y + 90 || pt1.y < pt2.y - 90&&deg==1)
			{
				cvLine(src, pt1, pt2, CV_RGB(392, 121, 155), 5, 8, 0);
			}
            deg=0;
		}

		roi_rect = cvRect(0, 0, src->width, src->height);
		cvSetImageROI((src), (roi_rect));
		cvSetImageROI((gray), (roi_rect));
		cvSetImageROI((dstCanny), (roi_rect));
		 
		 // Ekran acilir.
		 cvShowImage("Lanecrop",src);
          cvResetImageROI(src);
			cvClearMemStorage(storage);

           char key = (char)waitKey(10);
             
             // Buffer bosaltilir.
         cvReleaseImage(&dstCanny);
         cvReleaseImage(&halfFrame);
         cvReleaseImage(&gray);
         cvReleaseImage(&grey);
		cvReleaseImage(&smallImg);
          
          }
            // Buffer bosaltilir.
         cvReleaseMemStorage(&storage);
         cvReleaseImage(&dstCanny);
         cvReleaseImage(&halfFrame);
         cvReleaseImage(&gray);
         cvReleaseImage(&grey);
		cvReleaseImage(&smallImg);
		cvReleaseCapture(&cap);
		

}

void detect(IplImage *img)
{
	int flagCar;
   CvSize img_size = cvGetSize(img);
    CvSeq *object = cvHaarDetectObjects(
    img,
    cascade,
    storage,
    1.1, //Scale factor
    3,    // Minimum neighbours
    CV_HAAR_DO_CANNY_PRUNING, 
    cvSize(50,50),
    img_size 
    );
      string message="Az Yogun";
	CvPoint pt2 = cvPoint(10, 500);//nokta
	//olcek
	double hscale = 1.0;
	double vscale = 0.8;
	//keskinlik
	double shear = 0.2;
	int tickness2 = 1;
	int line_type = 8;

	// Araba sayisi icin flag tutuldu.
	flagCar=0;
	string colorStr;
	
		CvFont font1;
	cvInitFont(&font1, CV_FONT_HERSHEY_DUPLEX, hscale, vscale, shear, tickness2, line_type);
	
	const char * msg;
	// Ekranda 3 aractan az var ise
	if (object->total < 3){
	
		 msg = "Az Yogun";
	}
	// 3 arac var ise	
	else if (object->total == 3){
		 msg = "Orta Yogun";
	}
	// Daha fazla arac var ise
	else{
		 msg = "Cok Yogun";
	}
	cout << msg;
	cvPutText(img, msg, pt2, &font1, CV_RGB(250, 0, 0));
	

	    for(int i = 0 ; i < ( object ? object->total : 0 ) ; i++)	
  		{
 	   CvRect *r = (CvRect*)cvGetSeqElem(object, i);
 	   cvRectangle(img,
 			     cvPoint(r->x, r->y),
      			cvPoint(r->x + r->width, r->y + r->height),
     			 CV_RGB(255, 0, 0), 2, 8, 0);
      
 			cout<<"car";
 			// Color alma
 			colorStr= getCarColor(r->x, r->y, r->x + r->width, r->y + r->height, img);
 			
 			cout << colorStr << endl;
 		}

 		CvFont font;
 		double hScale=1.0;
 		double vScale=1.0;
 		int lineWidth=1;
 		cvInitFont(&font,CV_FONT_HERSHEY_SIMPLEX,hScale,vScale,0,lineWidth);
 		
 		
  		cvPutText(img, colorStr.c_str(), cvPoint(200,80), &font, Scalar(30, 20, 100));
  
         cout << "***" << endl;
}

// Threshold icin gerekli 2 hesaplama
// Mean  ve std 
// Ortalama ve standart sapma
double imageMean(IplImage * inp){
	
	Mat src=cvarrToMat(inp);
	long sum = 0;

	for (int i = 0; i < src.rows; ++i)
	{
		for (int j = 0; j < src.cols; j++)
		{
			sum += src.at<uchar>(i,j);
		}
	}
	return sum / (double)(src.cols * src.rows);

}

double imageStd(IplImage * inp, double mean){

	double sum = 0;
	Mat src=cvarrToMat(inp);
     			
		
	for (int i = 0; i < src.rows; ++i)
	{
		for (int j = 0; j < src.cols; j++)
		{
			sum += pow(src.at<uchar>(i,j)-mean,2);
		}
	}
	
	double var = sum / (double)(src.cols * src.rows);
	//cout << sqrt(var)<< "\n";
	return sqrt(var);
}

void *blink(void * arg){
	int counter=3000;
	for (;;){	
		if(flag == 1){
			digitalWrite(0, HIGH);
			delay(counter) ;
		}
		else{
			digitalWrite(0,  LOW);
			delay(counter) ;
		}
 	}
}
string getCarColor(int startX, int startY, int width, int height, IplImage* inp){
	
	IplImage* imgHSV = cvCreateImage(cvGetSize(inp), IPL_DEPTH_8U, 3);
	cvCvtColor(inp, imgHSV, CV_BGR2HSV);
	int colorVals[5]={0};
	int h=0, s=0, v=0;
	int max=0;
	for(int y=startY; y<height; ++y){
		uchar* ptr = (uchar*)(inp->imageData + y * inp->widthStep);
		for(int x=startX; x<width; ++x){
			h = ptr[3 * x]; //hue
			s = ptr[3 * x + 1]; //saturation
			v = ptr[3 * x + 2]; //value
			//cout << "h: " << h << "s: " << s << "v: " << v << endl;
			if(h>=0 && h<=15 && s>=40 && s<=100 && v>=0 && v<=100)
				colorVals[1] = colorVals[1] + 1; //red index 1
			else if(h>=170 && h<=180 && s>=40 && s<=100 && v>=0 && v<=100)
				colorVals[1] = colorVals[1] + 1; //red index 1
			else if(h>=20 && h<=30 && s>=10 && s<=255 && v>=0 && v<=255)
				colorVals[2] = colorVals[2] + 1; //yellow index 2
			else if(h>=110 && h<=115 && s>=40 && s<=100 && v>=50 && v<=100)
				colorVals[3] = colorVals[3] + 1; //blue index 3
			else if(h>=0 && h<=180 && s>=0 && s<=255 && v>=200 && v<=255)
				colorVals[0] = colorVals[0] + 1; //white index 0
			else if(h>=0 && h<=180 && s>=0 && s<=255 && v>=0 && v<=15)
				colorVals[4] = colorVals[4] + 1; //black index 4
		}
	}
	//colorVals[0] -= 500;
	for(int i=0; i<5; ++i){
		if(colorVals[i] > max)
			max = i;
	}
	
	if(max == 0)
		return "white";
	if(max == 1)
		return "red";
	if(max == 2)
		return "yellow";
	if(max == 3)
		return "blue";
	if(max == 4)
		return "black";
	
	return "color not found";
}


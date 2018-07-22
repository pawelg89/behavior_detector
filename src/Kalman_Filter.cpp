#include "..\..\stdafx.h"
#include "..\includes\Kalman_Filter.h"
#include <opencv2/highgui/highgui.hpp>
#include <opencv/cv.h>

using namespace cv;

Kalman_Filter::Kalman_Filter(void)
{
	KF.init(4,2,0);
	state.create(4,1);
	processNoise.create(4, 1, CV_32F);
	measurement.create(2,1);
	measurement.setTo(Scalar(0));
	active = true;
}


Kalman_Filter::~Kalman_Filter(void)
{
}


void Kalman_Filter::Initialize(CvPoint pt)
{
		KF.statePre.at<float>(0) = static_cast<float>(pt.x);
		KF.statePre.at<float>(1) = static_cast<float>(pt.y);
		KF.statePre.at<float>(2) = 0.0;
		KF.statePre.at<float>(3) = 0.0;
		KF.transitionMatrix = *(Mat_<float>(4, 4) << 1,0,2,0,   0,1,0,2,  0,0,1,0,  0,0,0,1);//TIME!!
		
        setIdentity(KF.measurementMatrix);
        setIdentity(KF.processNoiseCov, Scalar::all(1e-4));
        setIdentity(KF.measurementNoiseCov, Scalar::all(1e-1));
        setIdentity(KF.errorCovPost, Scalar::all(.1));
}
void Kalman_Filter::Action(CvPoint pt)
{
	Mat prediction = KF.predict();
  Point predictPt(static_cast<int>(prediction.at<float>(0)),static_cast<int>(prediction.at<float>(1)));
			
  measurement(0) = static_cast<float>(pt.x);
	measurement(1) = static_cast<float>(pt.y);
		
	measPt.x = static_cast<int>(measurement(0));
	measPt.y = static_cast<int>(measurement(1));
	posv.push_back(measPt);

	Mat estimated = KF.correct(measurement);
	statePt.x = static_cast<int>(estimated.at<float>(0));
	statePt.y = static_cast<int>(estimated.at<float>(1));
	kalmanv.push_back(statePt);
			
  randn( processNoise, Scalar(0), Scalar::all(sqrt(KF.processNoiseCov.at<float>(0, 0))));
	state = KF.transitionMatrix*state + processNoise;
}
void Kalman_Filter::print(IplImage *img)
{/* Routine for drawning path (still in development)
		for (int i = 1; i < posv.size()-1; i++) 
		{
			cvLine(img, posv[i], posv[i+1], Scalar(255,255,0), 1);
		}
		for (int i = 1; i < kalmanv.size()-1; i++) {
			cvLine(img, kalmanv[i], kalmanv[i+1], Scalar(0,255,0), 1);
		}*/
		//drawing circles	
		cvDrawCircle(img,statePt,5,Scalar(255,255,255));
		cvDrawCircle(img,measPt,5,Scalar(0,0,255));
}
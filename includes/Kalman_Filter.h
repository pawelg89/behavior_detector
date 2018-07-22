#pragma once
#include <opencv2/highgui/highgui.hpp>
#include <opencv/cv.h>
class Kalman_Filter
{
public:
	bool active;
	cv::KalmanFilter KF;
	cv::Mat_<float> state;
    cv::Mat processNoise;
    cv::Mat_<float> measurement;
	cv::Point statePt;
	cv::Point measPt;
	std::vector<cv::Point> posv;
	std::vector<cv::Point> kalmanv;
	Kalman_Filter(void);
	~Kalman_Filter(void);
	/*Set up Kalman Filter*/
	void Initialize(CvPoint pt);
	/*Estimates next object position*/
	void Action(CvPoint pt);
	/*print object position*/
	void print(IplImage* img);
};


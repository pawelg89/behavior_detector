#pragma once
#include <opencv2/highgui/highgui.hpp>
#include <opencv/cv.h>
//#include "stdafx.h"
#include "Kalman_Filter.h"
#include "BehaviorFilter.h"
#include "StructuresSM.h"
//
//using namespace std;
//using namespace cv;

class detected_object
{
private:
	//Ja chce miec tutaj to miejsce w moim kodzie
public:
	vector<BehaviorFilter*> bFilter;	
	vector<bool> eventSaved;
	string message;
	bool border;
	bool human;
	bool prediction_state;
	int prediction_life_time;
	int number;
	bool is_moving;
	double last_x;
	double last_z;
	double speed;
	double x_movement;
	double y_movement;
	bool direction_estimated;
	bool is_close_to;
	int camera;
	int outside_counter;
	int inside_counter;
	bool is_inside_restricted_area;
	Kalman_Filter *KFilter;
	cv::Rect rect;
	cv::Point current_pos;
	cv::Point next_pos;
	detected_object(void);
	detected_object(detected_object* obj);
	void calc_speed(double x, double z, int timex);
	~detected_object(void);

	int history_counter;
	vector<detected_object*>marged_objects;
	vector<double> pos_x;
    vector<double> pos_z;
	vector<double> time;
	vector<PointNorm> behDescr;

	bool detect_movement();
	inline	bool isinside(int);
	inline	bool isinside2(int);
	void estimate_direction();
	
	//Behavior Functions
	void CheckBehavior(/*std::vector<cv::Rect> vRect*/);
	vector<int> DetectedBehaviors();
	vector<bool> IsFound();
	void SetObjNumber();
	void ShowBehaviorStates(Mat frame);
};


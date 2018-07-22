#pragma once
#include <iostream>
#include <opencv\cv.h>
#include <opencv2\highgui\highgui.hpp>
#include <vector>

#include "detected_object.h"
class marked_frame
{
public:
	int min_x;
	int max_x;
	int min_y;
	int max_y;
	bool shape_closed;
	int width;
	int height;
	
	int points_counter;
	marked_frame(int w, int h);
	~marked_frame(void);
	
	/*Draw makred points*/
	void draw_circles(IplImage *image, std::vector<cv::Point2f> vec);
	/*New function which sets all pixels outside marked area to zero. (Prevents false detection outside area)*/
	void draw_shape(IplImage* image, std::vector<cv::Point2f> Points);
	/*returns true if objects is inside protected_area*/
	bool is_inside(detected_object*, std::vector<cv::Point2f>);
};


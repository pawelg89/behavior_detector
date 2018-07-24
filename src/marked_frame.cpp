#pragma once
#include "..\includes\marked_frame.h"
// STL includes
#include <iostream>
#include <vector>
// OpenCV includes
#include <opencv\cv.h>
#include <opencv2\highgui\highgui.hpp>
// Project includes
#include "..\..\stdafx.h"
#include "..\includes\detected_object.h"

namespace bd {
using namespace std;
using namespace cv;

marked_frame::marked_frame(int w, int h) {
  points_counter = 0;
  min_x = 10000;
  max_x = 0;
  min_y = 10000;
  max_y = 0;
  shape_closed = false;
  width = w;
  height = h;
}

marked_frame::~marked_frame(void) {}

void marked_frame::draw_shape(IplImage *image, std::vector<Point2f> Points) {
  if (abs(Points[(int)Points.size() - 1].x - Points[0].x) < 15 &&
      abs(Points[(int)Points.size() - 1].y - Points[0].y) < 15) {
    min_x = 10000;
    max_x = 0;
    min_y = 10000;
    max_y = 0;
    for (int i = 0; i < (int)Points.size(); i++) {
      line((Mat)image, Points[i], Points[(i + 1) % (int)Points.size()],
           cvScalar(0, 0, 255), 1, 8, 0);
    }
    CvPoint point;
    for (int i = 0; i < (int)Points.size(); i++) {
      point = Points[i];
      if (min_x > point.x) min_x = point.x;
      if (max_x < point.x) max_x = point.x;
      if (min_y > point.y) min_y = point.y;
      if (max_y < point.y) max_y = point.y;
    }
    shape_closed = true;
  } else {
    for (int i = 0; i < (int)Points.size() - 1; i++) {
      line((Mat)image, Points[i], Points[i + 1], cvScalar(0, 0, 128), 1, 8, 0);
    }
    shape_closed = false;
  }
}

void marked_frame::draw_circles(IplImage *image, std::vector<Point2f> vec) {
  for (int i = 0; i < (int)vec.size(); i++) {
    CvPoint pt2 = vec[i];
    cvCircle(image, pt2, 1, Scalar(255, 0, 255, 200), 2);
  }
}

bool marked_frame::is_inside(detected_object *object,
                             std::vector<cv::Point2f> Points) {
  Mat mask = Mat::zeros(Size(width, height), CV_8UC1);
  bool point_inside = false;
  for (int i = 0; i < (int)Points.size(); i++) {
    CvPoint pt1 = Points[i];
    CvPoint pt2 = Points[(i + 1) % (int)Points.size()];
    line(mask, pt1, pt2, cvScalar(255), 1, 8);
  }

  for (int x = 0; x < width; x++) {
    for (int y = 0; y < height; y++) {
      while (mask.at<unsigned char>(Point(x, y)) == 255 &&
             point_inside == false) {
        y++;
        point_inside = true;
      }

      while (point_inside == true &&
             mask.at<unsigned char>(Point(x, y)) == 255) {
        y++;
        point_inside = false;
      }

      if (point_inside && x == object->next_pos.x && y == object->next_pos.y) {
        return true;
      }
    }
    point_inside = false;
  }
  return false;
}
}  // namespace bd
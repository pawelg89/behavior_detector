#pragma once
// OpenCV includes
#include <opencv/cv.h>
#include <opencv2/highgui/highgui.hpp>
// Project includes
#include "BehaviorFilter.h"
#include "Kalman_Filter.h"
#include "StructuresSM.h"

namespace bd {

class detected_object {
 public:
  std::vector<BehaviorFilter*> bFilter;
  std::vector<bool> eventSaved;
  std::string message;
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
  Kalman_Filter* KFilter;
  cv::Rect rect;
  cv::Point current_pos;
  cv::Point next_pos;
  detected_object(void);
  detected_object(detected_object* obj);
  void calc_speed(double x, double z, int timex);
  ~detected_object(void);

  int history_counter;
  std::vector<detected_object*> marged_objects;
  std::vector<double> pos_x;
  std::vector<double> pos_z;
  std::vector<double> time;
  std::vector<PointNorm> behDescr;

  bool detect_movement();
  inline bool isinside(int);
  inline bool isinside2(int);
  void estimate_direction();

  // Behavior Functions
  void CheckBehavior();
  std::vector<int> DetectedBehaviors();
  std::vector<bool> IsFound();
  /*Load list of descriptors to be used and create filter bank from them.
  Input:
    enable - bool; enables loading custom thresholds if set true.*/
  void LoadBehaviorFilters(bool enable = false);
  /*Update object number in BehaviorFilter.*/
  void SetObjNumber();
  /*Visualizes behavior states in frame*/
  void ShowBehaviorStates(cv::Mat frame);

  private:
  std::vector<std::string> LoadDescriptorsList();
};
}  // namespace bd
#pragma once
// STL includes
#include <fstream>
#include <list>
// OpenCV includes
#include <opencv\cv.h>
#include <opencv2\nonfree\nonfree.hpp>
// Project includes
#include "BehaviorDescription.h"
#include "BehaviorFilter.h"
#include "Helper.h"
#include "detected_object.h"

namespace bd {

class Convex {
 public:
  int morph_size;

  Convex(void);
  Convex(int frameWidth, int frameHeight, int camID);
  ~Convex(void);

  /*Silhuettes of Human Images Extraction and Localization Detector*/
  void SHIELD(cv::Mat frame, cv::Mat frame_gray, int view);
  /*Silhuettes of Human Images Extraction and Localization Detector*/
  void SHIELD(cv::Mat frame, cv::Mat frame_gray, bool creatingDescriptors);
  /*Function that creates a convex hull on a given silhuette*/
  void NCM(cv::Mat frame, cv::Mat frame_gray, bool visualize = true);
  /*Function that deals with visualizing the result of NCM() function*/
  void VisualizeNCM(cv::Mat frame);
  /*Function that allows second iteration of finding NCMs, by updating convex
   * hull*/
  int UpdateHull(int startHull, int endHull, std::vector<cv::Point> &hull,
                 std::vector<cv::Point> &contour, int NCMidx);

  /*Function that prepares input vectors for each object on frame.*/
  void BehaviorInput(cv::Mat frame, std::vector<std::vector<cv::Point>> hulls);
  /**/
  void BehaviorFiltersCheck(cv::Mat frame /*, vector<vector<Point>>hulls*/);

  //------- Zmienne i funkcje wykorzystywane podczas sledzenia obiektow -------
  std::vector<cv::Point> temp_pos;
  std::vector<cv::Rect> temp_rect;
  std::vector<detected_object *> detected_objects;
  /*Input vectors for each object on frame.*/
  std::vector<std::vector<PointNorm>> behDescr;
  BehaviorDescription *BD;

  int track_objects(IplImage *frame, int c);
  /*Background update if scene changes too much*/
  bool is_background_ok(int width, int height);

 private:
  const double A;
  const double AREA_TRESH;
  const double AH_TRESH;
  const double SALIENCE_TRESH;
  int global_counter_;
  int wait_time_;

  int method_;
  int method0_, method1_, method2_, method3_;
  int l_pkt_;
  int clears_counter_;

  char key_;

  // Klasa pomocnicza przeliczajaca wspolrzedne
  Helper *GHelp;
  
  std::vector<std::vector<cv::Point>> contours;
  std::vector<std::vector<cv::Point>> hulls;
  std::vector<cv::Vec4i> hierarchy;
  // Vector containing subvectors with indices of NCM points for each contour
  std::vector<std::vector<int>> NCMs;
  // Vector containing each contour area
  std::vector<double> contour_areas;

  // vector with center points for each hull
  std::vector<cv::Point> centers;

  void ClearVectors();

  /*Calculates the distance between Point C and a line segment containing points
   * A and B.*/
  double Geth(cv::Point A, cv::Point B, cv::Point C);

  inline double GetDist(cv::Point A, cv::Point B);

  cv::Point ChangeCoords(cv::Point ox, cv::Point p);

  double GetAlfa(cv::Point p1, cv::Point prePt);

  double GetTheta(double x, double y, double alfa);
  
  /*Captures mouse clicks when manually choosing silhouettes for descriptors*/
  void HandleMouseCallbacks(cv::Mat frame);

  void SelectContour(cv::Mat frame);

  void SaveContourROI(cv::Mat frame, int idx);

  void SaveDescriptor();

  /*Function to save image containing new detected behavior.*/
  void SaveDetectedBehaviors(detected_object *obj, cv::Mat frame) const;

  /*Visualize contour parts that build up into a human*/
  void ShowContourElements(int rows, int cols);

  bool is_close_to(int master_index, int slave_index);
};
}  // namespace bd
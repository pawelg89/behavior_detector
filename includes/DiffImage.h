#pragma once
#include <string>
// OpenCV includes
#include <opencv\cv.h>
#include <opencv2\highgui\highgui.hpp>
// Project includes
#include "Convex.h"
#include "marked_frame.h"

namespace bd {

constexpr const char* kProcessingModes[] = {"normal", "training_auto", "training_manual"};
/* Numbering used here keeps it consistent with old numbering. */
enum class ProcessingMode { kNormal = 5, kTrainingAuto = 8, kTrainingManual = 7 };
bool IsProcessingMode(const std::string &mode_str);
ProcessingMode ToProcessingMode(const std::string &mode_str);
std::string ToString(const ProcessingMode &mode_en);

class DiffImage {
 public:
  DiffImage(const std::string& file);
  ~DiffImage(void);

  /*Start point of application, using IP camera.*/
  void DiffImageAction2();

  void TrackObjects3D(cv::Mat Frame, std::vector<Convex*> blob_vec,
                      std::vector<std::vector<cv::Point2f>>& marker_coord,
                      int view);
  void RestrictedAreaCheck(IplImage* frame, std::vector<cv::Point2f>& vec,
                           int camera, std::vector<cv::Mat> H_f,
                           std::vector<std::vector<cv::Point2f>>& marker_coord,
                           std::vector<Convex*> blob_vec);

  int iterations;
  const std::string file_name;
  marked_frame* MFrame;

 private:
  cv::Mat fore;
  cv::Mat frame;
};
}  // namespace bd
#pragma once
#include <atomic>
#include <vector>
// OpenCV includes
#include <opencv\cv.h>
#include <opencv2\core\core.hpp>
#include <opencv2\highgui\highgui.hpp>
#include <opencv2\video\background_segm.hpp>
#include "..\..\stdafx.h"

namespace bd {

class GCapture {
 public:
  GCapture(void);
  GCapture(int cams_number, bool async = true);
  ~GCapture(void);

  void QueryFrame(cv::Mat &frame, cv::Mat &fore, int i);

 private:
  void AsyncQuery(cv::Mat &frame, cv::Mat &fore, int i);
  void SyncQuery(cv::Mat &frame, cv::Mat &fore, int i);
  bool IsLiveCamera(const std::string &path);

  std::vector<std::string> pathDir_;
  std::vector<HANDLE> camThreads_;
};
}  // namespace bd
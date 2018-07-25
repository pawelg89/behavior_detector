#pragma once
#include <vector>
// OpenCV includes
#include <opencv\cv.h>
#include <opencv2\highgui\highgui.hpp>

namespace bd {

class Collector {
 public: // Singleton type
  static Collector &getInstance() {
    static Collector instance;
    return instance;
  }
  Collector(Collector const &) = delete;
  void operator=(Collector const &) = delete;

 public: // Exposed methods
  std::vector<std::pair<std::string, cv::Mat>> detections;
  
 private:
  Collector() = default;
  ~Collector() = default;

 private: // collected data
  
};

}  // namespace bd
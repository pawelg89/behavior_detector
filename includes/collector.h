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

 private:
  Collector() = default;
  ~Collector() = default;
 
public: // Exposed methods
  void AddData(const std::string &key, double data);
  void SaveData(const std::string &out_file = "collected_data.txt");

  std::vector<std::pair<std::string, cv::Mat>> detections;
  
 private: // collected data
  std::string CheckDataFilePath(const std::string &path);

  std::map<std::string, std::vector<double>> data_;
};

}  // namespace bd
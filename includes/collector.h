#pragma once
#include <functional>
#include <vector>
// OpenCV includes
#include <opencv\cv.h>
#include <opencv2\highgui\highgui.hpp>

namespace bd {

class Collector {
 public:  // Singleton type
  static Collector &getInstance() {
    static Collector instance;
    return instance;
  }
  Collector(Collector const &) = delete;
  void operator=(Collector const &) = delete;

 private:
  Collector() = default;
  ~Collector() = default;

 public:  // Exposed methods
  /*Add data under given key. Simple push_back.*/
  void AddData(const std::string &key, double data);
  /*Add data only if it hasn't been added yet.*/
  void AddDataOnce(const std::string &key, double data);
  /*Returns data vector from under specified 'key'. Empty vector if data does not exist.*/
  std::vector<double> Get(const std::string &key);
  /*Save all collected vectors to specified out_file, default: "collected_data.txt".*/
  void SaveData(const std::string &out_file = "collected_data.txt");

  std::vector<std::pair<std::string, cv::Mat>> detections;

 private:  // collected data
  std::string CheckDataFilePath(const std::string &path);

  std::map<std::string, std::vector<double>> data_;
};

}  // namespace bd
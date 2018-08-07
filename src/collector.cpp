#include "..\includes\collector.h"
#include <fstream>

namespace bd {

void Collector::AddData(const std::string &key, double data) {
  data_[key].push_back(data);
}

void Collector::SaveData(const std::string &out_file) {
  std::ofstream output(CheckDataFilePath(out_file));
  for (const auto &entries : data_) {
    output << entries.first << " ";
  }
  output << std::endl;
  for (const auto &entries : data_) {
    for (const auto &entry : entries.second) {
      output << entry << " ";
    }
    output << std::endl;
  }
}

std::string GetFilePath(const std::string &path) {
  auto separator = path.find('.');
  if (separator == std::string::npos) throw std::invalid_argument("Invalid file path.");
  return path.substr(0, separator);
}

std::string GetFileExtension(const std::string &path) {
  auto separator = path.find('.');
  if (separator == std::string::npos) throw std::invalid_argument("Invalid file path.");
  return path.substr(separator, path.size() - separator);
}

std::string Collector::CheckDataFilePath(const std::string &path) {
  std::ifstream checker(path);
  if (checker.is_open()) {
    checker.close();
    auto file_path = GetFilePath(path);
    auto file_extension = GetFileExtension(path);
    for (unsigned short i = 0; i < 1000; ++i) {
      std::string new_path = file_path + std::to_string(i) + file_extension;
      checker.open(new_path);
      if (!checker.is_open()) return new_path;
      checker.close();
    }
  } else {
    return path;
  }
  throw std::runtime_error("Exceeded allowed number of temporary test data collections.");
}

}  // namespace bd
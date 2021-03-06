#pragma once
// STL includes
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <mutex>
#include <string>

namespace bd {

#ifndef LOADDATA_TEMPLATE
#define LOADDATA_TEMPLATE
/*Function to load parameters from a file 'data_file'*/
template <class type>
bool load_data(const std::string &data_file, const std::string &data_name,
               type &data) {
  bool success = false;
  std::string temp_string = "";
  std::ifstream input;
  input.open(data_file);
  if (!input.is_open()) return success;

  while (!input.eof()) {
    input >> temp_string;
    if (temp_string == data_name) {
      input >> data;
      success = true;
      break;
    } else
      input >> temp_string;
  }
  input.close();

  return success;
}
#endif

/*Enum class representing possible logging levels. The higher the more
 * important, with OFF and ON reserved only for config file.*/
enum class LogLevel {
  kOn = 0,  //<!Used only in file to keep all logs ON.
  kDebug = 10,
  kMega = 20,
  kKilo = 30,
  kDetailed = 40,
  kDefault = 50,
  kSetup = 60,
  kWarning = 70,
  kError = 80,
  kCritical = 90,
  kOff = 100  //<!Used only in config to keep all logs OFF.
};

/*Conversion function LogLevel into std::string*/
std::string ToString(const LogLevel level);

/*Writes 'msg' to log file if 'level' if higher than reference level set in
  config. Printed in new line if 'new_line=true'; new line starts with date and
  'name' of a function that logs 'msg'.
  Returns: <int> 0 if successful or <int> -1 if 'level' was too low to print.*/
int LOG(const std::string &name, const std::string &msg, const LogLevel level,
        const bool new_line = true);

/*Writes 'msg' only ONCE to log file if 'level' if higher than reference level
  set in config. Printed in new line if 'new_line=true'; new line starts with
  date and 'name' of a function that logs 'msg'. Returns: <int> 0 if successful
  or <int> -1 if 'level' was too low to print or 'msg' has been printed already.*/
int LogOnce(const std::string &name, const std::string &msg,
            const LogLevel level, bool new_line = true);

class Logger {
 public: // Singleton type
  static Logger &getInstance() {
    static Logger instance;  // Guaranteed to be destroyed.
                             // Instantiated on first use.
    return instance;
  }
  Logger(Logger const &) = delete;
  void operator=(Logger const &) = delete;

  int LOG(const std::string &name, const std::string &msg, const LogLevel level,
          const bool new_line = true);

  void CloseStream();
  
 private:
  Logger();  // Constructor? (the {} brackets) are needed here.
  ~Logger();
  void Log(const std::string &name, const std::string &msg, const LogLevel level,
           const bool new_line = true);

  bd::LogLevel log_level_;
  std::ofstream log_;
  std::time_t rawtime_;
  struct tm *timeinfo_;
  std::string file_name_;
  std::mutex log_mutex_;
};

}  // namespace bd
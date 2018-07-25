#include "..\includes\logger.h"
// STL includes
#include <iomanip>
#include <iostream>
#include <strstream>

namespace bd {

std::string ToString(const LogLevel level) {
  switch (level) {
    case LogLevel::kCritical:
      return "critical";
    case LogLevel::kDebug:
      return "debug";
    case LogLevel::kDefault:
      return "default";
    case LogLevel::kDetailed:
      return "detailed";
    case LogLevel::kError:
      return "error";
    case LogLevel::kKilo:
      return "kilo";
    case LogLevel::kMega:
      return "mega";
    case LogLevel::kOff:
      return "off";
    case LogLevel::kOn:
      return "on";
    case LogLevel::kSetup:
      return "setup";
    case LogLevel::kWarning:
      return "warning";
    default:
      throw std::invalid_argument("ToString(): unsupported log level.");
  }
}

LogLevel ToLogLevel(const std::string &level) {
  if (level == "critical")
    return LogLevel::kCritical;
  else if (level == "debug")
    return LogLevel::kDebug;
  else if (level == "default")
    return LogLevel::kDefault;
  else if (level == "detailed")
    return LogLevel::kDetailed;
  else if (level == "error")
    return LogLevel::kError;
  else if (level == "kilo")
    return LogLevel::kKilo;
  else if (level == "mega")
    return LogLevel::kMega;
  else if (level == "off")
    return LogLevel::kOff;
  else if (level == "on")
    return LogLevel::kOn;
  else if (level == "setup")
    return LogLevel::kSetup;
  else if (level == "warning")
    return LogLevel::kWarning;
  else
    throw std::invalid_argument(
        "ToLogLevel(): unsupported log level as string.");
}

int LOG(const std::string &name, const std::string &msg, const LogLevel level,
        const bool new_line) {
  return Logger::getInstance().LOG(name, msg, level, new_line);
}
//-------------------------- SINGLETON LOGGER ---------------------------------

Logger::Logger() {
  // Load log_level from config
  std::string log_level_str = "default";
  if (load_data<std::string>("parameters.txt", "log_level", log_level_str))
    log_level_ = ToLogLevel(log_level_str);
  else
    log_level_ = LogLevel::kDefault;
  // Create log file
  time(&rawtime_);
  timeinfo_ = localtime(&rawtime_);
  file_name_ = "log_" + std::to_string(timeinfo_->tm_year + 1900) + "_" +
               std::to_string(timeinfo_->tm_mon + 1) + "_" +
               std::to_string(timeinfo_->tm_mday) + ".txt";

  if (!log_.is_open()) log_.open(file_name_, std::ios_base::app);
  this->LOG("Logger", "loaded.", LogLevel::kSetup);
}

Logger::~Logger() { log_.close(); }

int Logger::LOG(const std::string &name, const std::string &msg,
                const LogLevel level, const bool new_line) {
  if (level >= log_level_) {
    if (new_line) {
      log_ << "\n[" << std::setw(2) << std::setfill('0') << timeinfo_->tm_hour;
      log_ << ":" << std::setw(2) << std::setfill('0') << timeinfo_->tm_min;
      log_ << ":" << std::setw(2) << std::setfill('0') << timeinfo_->tm_sec;
      log_ << "] " << name << ": " << msg << std::flush;
    } else {
      log_ << msg << std::flush;
    }
    return 0;
  } else {
    return -1;
  }
}

}  // namespace bd
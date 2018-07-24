#include "..\includes\logger.h"
// STL includes
#include <ctime>
#include <fstream>
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

LogLevel LoadLogger(const LogLevel level) {
  LogLevel log_level = LogLevel::kDefault;
  std::string log_level_str = "default";
  if (load_data<std::string>("parameters.txt", "log_level", log_level_str))
    log_level = ToLogLevel(log_level_str);
  if (level == LogLevel::kOff || level == LogLevel::kOn) {
    std::string msg = "Invalid log level specified: " + ToString(level);
    throw std::invalid_argument(msg);
  }
  return log_level;
}

int LOG(const std::string &name, const std::string &msg, const LogLevel level,
        const bool new_line) {
  LogLevel log_level = LoadLogger(level);
  std::ofstream log;
  if (level >= log_level) {
    std::time_t rawtime;
    time(&rawtime);
    struct tm *timeinfo = localtime(&rawtime);

    char filename_buff[100] = "";
    std::ostrstream zapis(filename_buff, (int)sizeof(filename_buff),
                          std::ios::app);
    zapis << "log_" << timeinfo->tm_year + 1900 << "_" << timeinfo->tm_mon + 1
          << "_" << timeinfo->tm_mday << ".txt" << std::ends;

    if (!log.is_open()) log.open(filename_buff, std::ios_base::app);
    if (new_line) {
      log << "\n[" << std::setw(2) << std::setfill('0') << timeinfo->tm_hour;
      log << ":" << std::setw(2) << std::setfill('0') << timeinfo->tm_min;
      log << ":" << std::setw(2) << std::setfill('0') << timeinfo->tm_sec;
      log << "] " << name << ": " << msg << std::flush;
    } else {
      log << msg << std::flush;
    }
    log.close();
    return 0;
  } else {
    return -1;
  }
}
}  // namespace bd
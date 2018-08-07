#include <stdexcept>
#include <string>
#include "..\includes\logger.h"
#include "..\includes\timer.h"

namespace bd {
Timer::Timer(bool is_on)
    : is_on_(is_on),
      start_(high_resolution_clock::now()),
      mid_time_(start_),
      stop_(start_) {}

void Timer::Start() { start_ = high_resolution_clock::now(); }

void Timer::Stop() { stop_ = high_resolution_clock::now(); }

double Timer::Elapsed() {
  auto elapsed_ms = duration_cast<duration<double>>(stop_ - start_);
  if (elapsed_ms.count() < 0)
    throw std::runtime_error("bd::Timer: elapsed time negative.");
  return elapsed_ms.count();
}

std::string Timer::PrintElapsed(const std::string &msg, bool new_line) {
  stop_ = high_resolution_clock::now();
  last_elapsed = duration_cast<duration<double>>(stop_ - mid_time_).count();
  std::string message = msg + "(" + ToString(last_elapsed) + ") ";
  if (is_on_) LOG("Timer", message, LogLevel::kSetup, new_line);
  mid_time_ = high_resolution_clock::now();
  return message;
}

std::string Timer::ToString(double duration) {
  if (duration < 1.0) {    
    return std::to_string(duration * 1000) + "ms";
  } else if (duration < 60.0) {    
    return std::to_string(duration) + "s";
  } else {
    auto minutes = static_cast<int>(duration) / 60;
    auto seconds = duration - (minutes * 60);
    std::string msg = std::to_string(minutes) + "min ";
    msg += std::to_string(seconds) + "s";
    return msg;
  }
}

}  // namespace bd
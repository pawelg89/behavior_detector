#include "..\includes\signaler.h"
#include <iostream>
#include "..\includes\logger.h"

namespace bd {

Signaler::Signaler() : prev_path_("") { signals_.clear(); }

bool Signaler::CheckAndReset(const std::string &sig_name) {
  auto check = Check(sig_name);
  std::lock_guard<std::mutex> guard(signals_mutex_);
  if (check) signals_[sig_name] = false;
  return check;
}

void Signaler::SignalPathChange(const std::string &path,
                                const std::string &sig_name) {
  if (path.size() < 8)
    throw std::runtime_error("Signaler::SignalPathChange too short path given.");
  auto comp_substr = path.substr(0, path.find_last_of('/'));
  while (Check(sig_name))
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  std::lock_guard<std::mutex> guard(signals_mutex_);
  signals_[sig_name] = (prev_path_ != comp_substr);
  if (prev_path_ != comp_substr) 
    LOG("Signaler", comp_substr, LogLevel::kDetailed);
  
  prev_path_ = comp_substr;
}

bool Signaler::Check(const std::string &sig_name) {
  auto obj = signals_.find(sig_name);
  if (obj == signals_.end()) return false;
  return obj->second;
}

}  // namespace bd
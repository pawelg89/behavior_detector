#pragma once
#include <map>
#include <string>
#include <mutex>

namespace bd {

class Signaler {
 public: // Singleton type
  static Signaler &getInstance() {
    static Signaler instance;
    return instance;
  }
  Signaler(Signaler const &) = delete;
  void operator=(Signaler const &) = delete;

 private:
  Signaler();
  ~Signaler() = default;
 
public: // Exposed methods
  bool CheckAndReset(const std::string &sig_name);
  void SignalPathChange(const std::string &path, const std::string &sig_name);

 private: // internal methods
  bool Check(const std::string &sig_name);

  std::map<std::string, bool> signals_;
  std::mutex signals_mutex_;
  std::string prev_path_;
};

}  // namespace bd
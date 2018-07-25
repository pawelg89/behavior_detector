#pragma once
#include <chrono>
#include <cstdlib>
#include <ctime>
#include <string>

namespace bd {
using namespace std::chrono;
/*Class used to measure time elapsed from start to stop. Timer 'start' is set on
 * construction */
class Timer {
 public:
  Timer(bool is_on = true);
  ~Timer() = default;
  /*Set start time*/
  void Start();
  /*Set stop time*/
  void Stop();
  /*Print seconds elapsed between start and stop. Start is set to construction
   * time so its possible to do: 
   *    Timer obj;
   *    ... here do something ...
   *    auto elapsed_time = obj.Elapsed();**/
  double Elapsed();
  /*Print to LOG directly result of Elapsed()*/
  std::string PrintElapsed(const std::string &msg = "", bool new_line = true);

 private:
  std::string ToString(double duration);

  bool is_on_;
  high_resolution_clock::time_point start_;
  high_resolution_clock::time_point mid_time_;
  high_resolution_clock::time_point stop_;
};

}  // namespace bd
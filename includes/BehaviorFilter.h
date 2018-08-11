#pragma once
// OpenCV includes
#include <opencv\cv.h>
#include <opencv2\nonfree\nonfree.hpp>
// Project includes
#include "BehaviorState.h"
#include "StructuresSM.h"

namespace bd {

class BehaviorFilter {
 protected:
  /*Don't touch this!!*/
  std::vector<BehaviorState *> temp_StateHandles;

 private:
  BehaviorState *firstState;
  BehaviorState *currentState;
  std::vector<int> method;
  int lPkt;

 public:
  std::string behaviorDescription;
  int behaviorType;
  bool found;
  int objNumber;

  BehaviorFilter(void);
  BehaviorFilter(const std::string &path);
  BehaviorFilter(const std::string &path, int gs);
  ~BehaviorFilter(void);

  /*Default function to check behavior on current filter.*/
  void Check(std::vector<PointNorm> input);
  /*Function to check behavior used if current filter is searching for fainting
   * people.*/
  void Check(std::vector<PointNorm> input, bool isMoving);

  std::vector<int> BehaviorType();
  int GetCurrentStateNumber();
  void Rewrite(char *path, double thresholdNew);
  void SetFirstState(BehaviorState *first);
  void SetThreshold(double thresh);
  void SetBehaviorType(int type, std::string message);
  void Save(char *path);
  BehaviorState *GetFirstState();
  /*Function that lets you join another behavior filter as an alternate route.*/
  void Append(BehaviorFilter *appFilter);

  friend class DataBase;
};
}  // namespace bd
#pragma once
// STL includes
#include <vector>
// OpenCV includes
#include <opencv\cv.h>
// Project includes
#include "StructuresSM.h"

namespace bd {

class BehaviorDescription {
 public:
  std::vector<std::vector<PointNorm>> descriptor;
  std::vector<int> v_sizes;
  int *sizes;
  bool *map;

  BehaviorDescription(void);
  ~BehaviorDescription(void);

  void SaveBehaviorDescriptor(char *descriptor_name /*,int method = 0*/);
};
}  // namespace bd
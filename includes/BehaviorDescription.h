#pragma once
// STL includes
#include <vector>
// OpenCV includes
#include <opencv\cv.h>
// Project includes
#include "StructuresSM.h"

namespace bd {

struct BehaviorType {
  int id;
  std::string name;
  std::string cntr_file;
  BehaviorType() = default;
  BehaviorType(int id_in, std::string name_in)
      : id(id_in), name(std::move(name_in)), cntr_file{name + "Num.txt"} {}
  void UpdateCounterFile() { cntr_file = name + "Num.txt"; }
};

class BehaviorDescription {
 public:
  BehaviorDescription(void);
  ~BehaviorDescription(void);

  void SaveBehaviorDescriptor(int id = -1, const std::string &name = "unknown");
  void Clear();
  
  std::vector<std::vector<PointNorm>> descriptor;
  std::vector<int> v_sizes;
  int *sizes;
  bool *map;

 private:
   void SaveCounter(const std::string &file_name, int counter);
   int ReadCounter(const std::string &file_name);
   inline std::string MakeDescriptorName(const std::string &behavior_name, int counter);
   BehaviorType FindBehavior(int id);
   BehaviorType FindBehavior(const std::string &bahavior_name);
   void LoadBehaviorList();
   void AddBehaviorToList(BehaviorType beh_type);
   std::pair<bool, BehaviorType> GetBehaviorType(int id, const std::string &name);
   std::string AskForBehName(int id);

   std::vector<BehaviorType> behavior_types_;

};
}  // namespace bd
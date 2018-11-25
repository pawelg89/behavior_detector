#include "../includes/StructuresSM.h"

namespace bd {

std::string to_string(const PointNorm &pt) {
  return std::string("(" + std::to_string(pt.x) + "," + std::to_string(pt.y) + ")");
}

std::string to_string(const std::vector<PointNorm> &pts) {
  std::string pts_str = "";
  for (const auto el : pts) pts_str += to_string(el);
  return pts_str;
}

}  // namespace bd
#pragma once

#include <string>
#include <vector>

#include <opencv\cv.h>

namespace bd {
// Structure used in Smart Monitor project.
#ifndef POINTNORM
#define POINTNORM

class PointNorm {
 public:
  double x;
  double y;

  PointNorm(void) {}
  PointNorm(double a, double b) {
    x = a;
    y = b;
  }
  ~PointNorm() {}
};

std::string to_string(const PointNorm &pt);
std::string to_string(const std::vector<PointNorm> &pts);
#endif

enum CAM_PARAM {
  CAM_PARAM_IMAGE_WIDTH = 1,
  CAM_PARAM_IMAGE_HEIGHT = 2,
  CAM_PARAM_DX = 3,
  CAM_PARAM_DY = 4,
  CAM_PARAM_ALPHA = 5,
  CAM_PARAM_FOCAL = 6,
  CAM_PARAM_CAM_HEIGHT = 7
};

}  // namespace bd
#pragma once
#include <opencv/cv.h>
#include <vector>
#include "Convex.h"

namespace bd {

void match_objects(std::vector<std::vector<cv::Point2f>> obj_positions,
                   std::vector<std::vector<double>> &matches, int view,
                   std::vector<double> path);

void sort_matches(std::vector<std::vector<double>> &matches);

void filter_matches(std::vector<std::vector<double>> &matches, double dist);

void label_objects(std::vector<std::vector<double>> &matches,
                   std::vector<Convex *> blob_vec);

void match_text(std::vector<std::vector<double>> &matches,
                std::vector<Convex *> blob_vec, int a, int b);
}  // namespace bd
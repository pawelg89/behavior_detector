#include "..\includes\BehaviorState.h"
#include "..\..\stdafx.h"
#include "..\includes\logger.h"


#include <opencv\cv.h>
#include <opencv2\highgui.hpp>

namespace bd {
  namespace {
inline int BS_LOG(const std::string &msg, const LogLevel level, bool new_line = true) {
  return LOG("BehaviorState", msg, level, new_line);
}
int BS_LogOnce(const std::string &msg, const LogLevel level,
               bool new_line = true) {
  return LogOnce("BehaviorState", msg, level, new_line);
}
}  // namespace
//----------------- C O N S T R U C T O R S -----------------------------------
BehaviorState::BehaviorState(double thresh, int method, int lPkt) {
  lastState = false;
  if (!load_data("parameters.txt", "visualize_state_compare", visualize)) visualize = false;
  if (!load_data("parameters.txt", "show_debug_messages", show_debug_msgs)) show_debug_msgs = false;

  idleCounter = 0;
  threshold = thresh;
  behType = 0;
  if (!load_data("parameters.txt", "faintCounter", faintCounter))
    faintCounter = 80;
  if (!load_data("parameters.txt", "statsON", statsON)) statsON = false;
  std::string message = "faintCounter=" + std::to_string(faintCounter);
  message += (statsON) ? ", statsON=true" : ", statsON=false";
  message += ", method=" + std::to_string(method);
  message += ", threshold=" + std::to_string(thresh);
  message += ", BehaviorState(double, int, int)";
  BS_LogOnce(message, LogLevel::kSetup);

  // switch(method)
  //{
  // case 0:
  //	methodIdxStart = 1;
  //	methodIdxStop = 5;
  //	break;
  // case 1:
  //	methodIdxStart = 6;
  //	methodIdxStop = 10;
  //	break;
  // case 2:
  //	methodIdxStart = 11;
  //	methodIdxStop = 10+lPkt+1;
  //	break;
  // case 3:
  //	methodIdxStart = 11+lPkt+2;
  //	methodIdxStop = 10+lPkt;
  //	break;
  // default:
  //	methodIdxStart = 1;
  //	methodIdxStop = 5;
  //}
}

BehaviorState::BehaviorState(std::vector<PointNorm> accInput, bool isLast,
                             double thresh, int method, int lPkt) {
  acceptableInput = accInput;
  lastState = isLast;
  if (!load_data("parameters.txt", "visualize_state_compare", visualize)) visualize = false;
  if (!load_data("parameters.txt", "show_debug_messages", show_debug_msgs)) show_debug_msgs = false;

  threshold = thresh;
  idleCounter = 0;
  behType = 0;
  if (!load_data("parameters.txt", "faintCounter", faintCounter))
    faintCounter = 80;
  if (!load_data("parameters.txt", "statsON", statsON)) statsON = false;
  switch (method) {
    case 0:
      methodIdxStart = 1;
      methodIdxStop = 5;
      break;
    case 1:
      methodIdxStart = 6;
      methodIdxStop = 10;
      break;
    case 2:
      methodIdxStart = 11;
      methodIdxStop = 10 + lPkt + 1;
      break;
    case 3:
      methodIdxStart = 11 + lPkt + 2;
      methodIdxStop = (int)acceptableInput.size();
      break;
    default:
      methodIdxStart = 1;
      methodIdxStop = 5;
  }
  std::string message = "faintCounter=" + std::to_string(faintCounter);
  message += (statsON) ? ", statsON=true" : ", statsON=false";
  message += ", method=" + std::to_string(method);
  message += ", threshold=" + std::to_string(thresh);
  message += ", BehaviorState(vector<PointNorm>, bool, double, int, int)";
  BS_LogOnce(message, LogLevel::kSetup);
}

BehaviorState::BehaviorState(std::vector<PointNorm> accInput,
                             std::vector<BehaviorState*> nextstts, bool isLast,
                             double thresh, int method, int lPkt) {
  acceptableInput = accInput;
  nextStates = nextstts;
  lastState = isLast;
  if (!load_data("parameters.txt", "visualize_state_compare", visualize)) visualize = false;
  if (!load_data("parameters.txt", "show_debug_messages", show_debug_msgs)) show_debug_msgs = false;

  threshold = thresh;
  idleCounter = 0;
  behType = 0;
  if (!load_data("parameters.txt", "faintCounter", faintCounter))
    faintCounter = 80;
  if (!load_data("parameters.txt", "statsON", statsON)) statsON = false;
  switch (method) {
    case 0:
      methodIdxStart = 1;
      methodIdxStop = 5;
      break;
    case 1:
      methodIdxStart = 6;
      methodIdxStop = 10;
      break;
    case 2:
      methodIdxStart = 11;
      methodIdxStop = 10 + lPkt + 1;
      break;
    case 3:
      methodIdxStart = 11 + lPkt + 2;
      methodIdxStop = (int)acceptableInput.size();
      break;
    default:
      methodIdxStart = 1;
      methodIdxStop = 5;
  }
  std::string message = "faintCounter=" + std::to_string(faintCounter);
  message += (statsON) ? ", statsON=true" : ", statsON=false";
  message += ", method=" + std::to_string(method);
  message += ", threshold=" + std::to_string(thresh);
  message += ", BehaviorState(vector<PointNorm>, vector<BehaviorState*>, bool, double, int, int)";
  BS_LogOnce(message, LogLevel::kSetup);
}

BehaviorState::BehaviorState(std::vector<PointNorm> accInput,
                             std::vector<BehaviorState*> nextstts, bool isLast,
                             std::string sttDescr, double thresh, int method,
                             int lPkt) {
  acceptableInput = accInput;
  nextStates = nextstts;
  lastState = isLast;
  if (!load_data("parameters.txt", "visualize_state_compare", visualize)) visualize = false;
  if (!load_data("parameters.txt", "show_debug_messages", show_debug_msgs)) show_debug_msgs = false;
  stateDescription = sttDescr;

  threshold = thresh;
  idleCounter = 0;
  behType = 0;
  if (!load_data("parameters.txt", "faintCounter", faintCounter))
    faintCounter = 80;
  if (!load_data("parameters.txt", "statsON", statsON)) statsON = false;
  switch (method) {
    case 0:
      methodIdxStart = 1;
      methodIdxStop = 5;
      break;
    case 1:
      methodIdxStart = 6;
      methodIdxStop = 10;
      break;
    case 2:
      methodIdxStart = 11;
      methodIdxStop = 11 + lPkt + 1;
      break;
    case 3:
      methodIdxStart = 11 + lPkt + 2;
      methodIdxStop = (int)acceptableInput.size();
      break;
    default:
      methodIdxStart = 1;
      methodIdxStop = 5;
  }
  std::string message = "faintCounter=" + std::to_string(faintCounter);
  message += (statsON) ? ", statsON=true" : ", statsON=false";
  message += ", method=" + std::to_string(method);
  message += ", threshold=" + std::to_string(thresh);
  message += ", sttDescr=" + sttDescr;
  message += ", BehaviorState(vector<PointNorm>, vector<BehaviorState*>, bool, string, double, int, int)";
  BS_LogOnce(message, LogLevel::kSetup);
}

BehaviorState::~BehaviorState(void) {
  acceptableInput.clear();
  nextStates.clear();
}
//-----------------------------------------------------------------------------------------------

int BehaviorState::GetConnectionsNumber() {
  int connNum = 0;
  for (int i = 0; i < (int)this->nextStates.size(); i++) {
    connNum += nextStates[i]->GetConnectionsNumber();
  }
  return connNum + (int)this->nextStates.size();
}

void BehaviorState::MarkConnections(bool* map, int mapSize) {
  for (int i = 0; i < (int)this->nextStates.size(); i++) {
    map[this->sttNumber * mapSize + this->nextStates[i]->sttNumber] = true;
    this->nextStates[i]->MarkConnections(map, mapSize);
  }
}

void BehaviorState::SaveDescriptor(std::ostream& output) {
  for (int i = 0; i < (int)this->acceptableInput.size(); i++) {
    output.write((char*)&this->acceptableInput[i].x, sizeof(double));
    output.write((char*)&this->acceptableInput[i].y, sizeof(double));
  }
  for (int i = 0; i < (int)this->nextStates.size(); i++) {
    this->nextStates[i]->SaveDescriptor(output);
  }
}

void BehaviorState::SaveStatistic(std::vector<PointNorm> inputVector) {
  char filename_buff[100] = "";
  std::ostrstream zapis(filename_buff, (int)sizeof(filename_buff),
                        std::ios::app);
  CreateDirectoryA((LPCSTR) "Statistics", NULL);

  for (int ptNr = 0; ptNr < (int)inputVector.size(); ptNr++) {
    // Zapisanie wspolrzednej x
    zapis.seekp(0);
    zapis << "Statistics/beh" << this->behType << "_sttNr" << this->sttNumber
          << "_pt" << ptNr << "_x.tmp" << std::ends;
    std::fstream str(filename_buff, std::ios::out | std::ios::app);
    str << inputVector[ptNr].x << " ";
    str.close();

    // Zapisanie wspolrzednej y
    zapis.seekp(0);
    zapis << "Statistics/beh" << this->behType << "_sttNr" << this->sttNumber
          << "_pt" << ptNr << "_y.tmp" << std::ends;
    str.open(filename_buff, std::ios::out | std::ios::app);
    str << inputVector[ptNr].y << " ";
    str.close();
  }
}

//----------------------------- P U B L I C -----------------------------------

BehaviorState* BehaviorState::ChangeState(std::vector<PointNorm> inputVector,
                                          bool isMoving) {
  BS_LOG("ChangeState()", LogLevel::kKilo, true);
  if (statsON) SaveStatistic(inputVector);
  bool accepted = true;
  // Search best possible next state
  int chosenState = 0;
  double bestDistance = (double)INT_MAX;

  for (int i = 0; i < (int)nextStates.size(); ++i) {
    int mismatch = GetAcceptedMissmatchCount(i);
    if (this->show_debug_msgs) {
      std::string debug_msg = PrepareDebugMessage(inputVector, i);
    }
    MatchDistance match = MatchDescriptors(inputVector, i);

    if (match.distance <= match.assigned * nextStates[i]->threshold) {
      if (match.unassigned <= mismatch) {
        if (match.distance < bestDistance) {
          chosenState = i;
          bestDistance = match.distance;
        }
      } else {
        accepted = false;
        BS_LOG(descriptor_path + " REJECTED BY TOO MANY UNASSIGNED POINTS!",
               LogLevel::kMega);
      }
    } else {
      accepted = false;
      BS_LOG(descriptor_path + " REJECTED BY TOO BIG THRESHOLD!", LogLevel::kMega);
    }
  }
  // If possible move to that state, if not update idle counter.
  std::pair<bool, BehaviorState*> possible_state = 
    GoToState(std::pair<bool, int>{accepted, chosenState}, isMoving);
  if (possible_state.first) return possible_state.second;
  else return CheckIdleCounter();
}

void UniquePushBack(std::vector<PointNorm> &vec, const PointNorm &el) {
  bool is_unique = true;
  for (const auto& e : vec) {
    if (el.x == e.x && el.y == e.y) {
      is_unique = false;
      break;
    }
  }
  if (is_unique) vec.push_back(el);
}

MatchDistance BehaviorState::MatchDescriptors(
    const std::vector<PointNorm>& inputVector, const int i) {
  static cv::Mat bs_debug_img{cv::Size(640, 480), CV_8UC3, cv::Scalar(0,0,0)};
  const int idx_start = nextStates[i]->methodIdxStart;
  const int idx_stop = nextStates[i]->methodIdxStop;
  const int pt_count = idx_stop - idx_start;
  //---------------------------------------------------------------------------
  const PointNorm in_dim(inputVector[0].x, inputVector[0].y);
  const PointNorm descr_dim(nextStates[i]->acceptableInput[0].x, 
                            nextStates[i]->acceptableInput[0].y);
  //----------- PREPARING INPUT VECTOR AND DESCRIPTOR -------------------------
  std::vector<PointNorm> input; input.reserve(pt_count);
  for (int j = idx_start; j < idx_stop && j < (int)inputVector.size(); ++j) {
    UniquePushBack(input, inputVector[j]);
  }
  std::vector<PointNorm> descr; descr.reserve(pt_count);
  for (int j = idx_start; j < idx_stop; ++j) {
    UniquePushBack(descr, nextStates[i]->acceptableInput[j]);
  }
  //------------------ DISPLAY BOTH VECTORS -----------------------------------
  if (visualize) {
    bs_debug_img = {cv::Size(640, 480), CV_8UC3, cv::Scalar(0,0,0)};
    for (const auto& pt : descr) {
      auto nextSttPt = cv::Point(int(descr_dim.x * pt.x + 320), int(descr_dim.y * pt.y + 240));
      cv::circle(bs_debug_img, nextSttPt, 3, cv::Scalar(0,255,0));
    }
    for (const auto& pt : input) {
      auto inputPt = cv::Point(int(in_dim.x * pt.x + 320), int(in_dim.y * pt.y + 240));
      cv::circle(bs_debug_img, inputPt, 2, cv::Scalar(255,0,0));
    }
    cv::putText(bs_debug_img, descriptor_path, cv::Point(10, 10), CV_FONT_VECTOR0, 0.4, cv::Scalar(255,255,255));
    cv::imshow("all points", bs_debug_img);
  }
  //---------------------------------------------------------------------------
  const size_t in_size = input.size();
  const size_t descr_size = descr.size();
  const size_t unassigned_pts = std::abs((long long)in_size - (long long)descr_size);
  
  DistFieldMatrix dist_matrix = CreateDistanceFieldMatrix(input,descr);
  
  VecDistField result = CreatePairs(dist_matrix);

  double distance = 0.0;
  for (const auto& df : result) {
    distance += df.dist;
  }
  if (visualize) {
    Visualize(bs_debug_img, result, i, in_dim, descr_dim, distance, input, descr);
    if (show_debug_msgs) {
      std::cout << "input.size() = " << in_size << std::endl;
      std::cout << "descr.size() = " << descr_size << std::endl;
    }
    cv::putText(bs_debug_img, descriptor_path + " distance=" + std::to_string(distance),
                cv::Point(10, 10), CV_FONT_VECTOR0, 0.4, cv::Scalar(255,255,255));
    cv::imshow("matching", bs_debug_img);
    cv::waitKey();
  }
  return MatchDistance{distance, unassigned_pts, result.size()};
}

DistFieldMatrix BehaviorState::CreateDistanceFieldMatrix(const std::vector<PointNorm> &input, 
                                                         const std::vector<PointNorm> &descr) {
  const size_t in_size = input.size();
  const size_t descr_size = descr.size();
  DistFieldMatrix dist_matrix;
  dist_matrix.resize(in_size);
  for (size_t n = 0; n < in_size; ++n) {
    for (size_t m = 0; m < descr_size; ++m) {
      const double dist = GetDist(input[n], descr[m]);
      auto dist_field = DistField{n, m, dist};
      dist_matrix[n].push_back(dist_field);
    }
    std::sort(
        dist_matrix[n].begin(), dist_matrix[n].end(),
        [](const DistField& a, const DistField& b) { return a.dist < b.dist; });
  }
  std::sort(dist_matrix.begin(), dist_matrix.end(),
            [](const VecDistField& a, const VecDistField& b) {
              return a[0].dist < b[0].dist;
            });
  return dist_matrix;
}

VecDistField BehaviorState::CreatePairs(DistFieldMatrix &dist_matrix) {
  const size_t in_size = dist_matrix.size();
  VecDistField result; 
  for (size_t n = 0; n < in_size; ++n) {
    /* pick first free pair*/
    DistField closest_pair;
    for (const auto& df : dist_matrix[n]) {
      if (!df.picked) {
        closest_pair = df;
        closest_pair.picked = true;
        break;
      }
    }
    if (closest_pair.picked) result.push_back(closest_pair);
    else continue;
    /* mark elements with m == closest_pair.m in remaining rows*/
    for (size_t k = n + 1; k < in_size; ++k) {
      for (auto& df : dist_matrix[k]) {
        if (df.m == closest_pair.m) df.picked = true;
      }
    }
  }
  return result;
}

std::string BehaviorState::PrepareDebugMessage(const std::vector<PointNorm>& inputVector, 
                                               const int i) {
  std::string msg = "nextStates[" + std::to_string(i) + "]:" + to_string(nextStates[i]->acceptableInput);
  BS_LOG(msg, LogLevel::kDebug);
  msg = "inputVector[" + std::to_string(i) + "]:" + to_string(inputVector);
  BS_LOG(msg, LogLevel::kDebug);
  return "States being compared[" + std::to_string(i) + "]: ";
}

std::pair<bool, BehaviorState*>
BehaviorState::GoToState(std::pair<bool, int> next_state, bool is_moving) {
  auto accepted = next_state.first;
  auto chosen_state = next_state.second;
  std::pair<bool, BehaviorState*> result{false, NULL};
  if (accepted) {
    if (nextStates[chosen_state]->lastState == true && behType == 3) {
      if (!is_moving) {
        if (faintCounter <= 1) {
          idleCounter = 0;
          result.first = true;
          result.second = this->nextStates[chosen_state];
        } else {
          faintCounter--;
        }
      }
    } else {
      idleCounter = 0;
      result.first = true;
      result.second = this->nextStates[chosen_state];
    }
  }
  return result;
}

BehaviorState* BehaviorState::CheckIdleCounter() {
  if (idleCounter >= 70) {
    std::string msg =
        "Idle counter filled. Returning to the first state. Behavior: " +
        std::to_string(behType);
    BS_LOG(msg, LogLevel::kKilo);
    idleCounter = 0;
    return NULL;
  } else {
    if (this->sttNumber >= 1) {
      idleCounter++;
    }
    return this;
  }
}

void BehaviorState::Visualize(cv::Mat& debug_img, const VecDistField& result, const int i,
                              const PointNorm &in_dim, const PointNorm &descr_dim, const double distance,
                              const std::vector<PointNorm> &input, const std::vector<PointNorm> &descr) {
  int counter = 0;
  for (const auto& df : result) {
    auto color = cv::Scalar(0,255,0);
    if (df.dist > nextStates[i]->threshold) {
        color = cv::Scalar(0,0,255);
    }
    if (show_debug_msgs) std::cout << "(" << df.n << ", " << df.m << ")";
    // Visualize input point (BLUE)
    auto inputPt = cv::Point(int(in_dim.x * input[df.n].x + 320), 
                             int(in_dim.y * input[df.n].y + 240));
    cv::circle(debug_img, inputPt, 3, cv::Scalar(255,0,0));
    // Visualize descriptor point (GREEN)
    auto descrPt = cv::Point(int(descr_dim.x * descr[df.m].x + 320), 
                             int(descr_dim.y * descr[df.m].y + 240));
    cv::circle(debug_img, descrPt, 4, cv::Scalar(0,255,0));
    if (show_debug_msgs) {
      std::strstream msg;
      msg << "(" << df.n << ", " << df.m << "): " << df.dist << " | " 
          << "(" << inputPt.x << ", " << inputPt.y << ") (" 
          << descrPt.x << ", " << descrPt.y << ")" << std::ends;
      cv::putText(debug_img, msg.str(), cv::Point(10, 10 + 17 * ++counter),
                  CV_FONT_VECTOR0, 0.4, cv::Scalar(50, 255, 50));    
    }
    cv::line(debug_img, inputPt, descrPt, color);
    //cv::imshow("matching", debug_img);
    //cv::waitKey();
  } 
  if (show_debug_msgs) std::cout << std::endl;
  //cv::imshow("matching", debug_img);
  //cv::waitKey();
}

int BehaviorState::GetAcceptedMissmatchCount(int i) {
  int mistakes = 0;
  switch (methodIdxStart) {
    case 11:
      if (behType == 6 || behType == 7)
        mistakes = (int)(0.15 * (nextStates[i]->methodIdxStop -
                                 nextStates[i]->methodIdxStart) +
                          0.5);
      else
        mistakes = (int)(0.35 * (nextStates[i]->methodIdxStop -
                                 nextStates[i]->methodIdxStart) +
                          0.5);
      break;
    case 6:
      if (behType == 6 || behType == 7)
        mistakes = 0;
      else
        mistakes = 1;
      break;
    case 1:
      if (behType == 6 || behType == 7)
        mistakes = 0;
      else
        mistakes = 1;
      break;
    default:
      mistakes = (int)(0.12 * (nextStates[i]->methodIdxStop -
                               nextStates[i]->methodIdxStart) +
                        0.5);
      break;
  }
  return mistakes;
}

inline double BehaviorState::GetDist(PointNorm A, PointNorm B) {
  return sqrt((A.x - B.x) * (A.x - B.x) + (A.y - B.y) * (A.y - B.y));
}

void BehaviorState::SetNextStates(std::vector<BehaviorState*> nextStts) {
  nextStates = nextStts;
}

void BehaviorState::SetIdxStop(int idxStop) { this->methodIdxStop = idxStop; }

}  // namespace bd
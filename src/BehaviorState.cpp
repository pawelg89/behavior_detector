#include "..\includes\BehaviorState.h"
#include "..\..\stdafx.h"
#include "..\includes\logger.h"


#include <opencv\cv.h>
#include <opencv2\highgui.hpp>

namespace bd {
  namespace {
int BS_LOG(const std::string &msg, const LogLevel level, bool new_line = true) {
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

  for (int i = 0; i < (int)nextStates.size(); i++) {
    double tempDistance = 0.0;
    int mistakes = GetAcceptedMissmatchCount(i);
    std::string debug_msg = PrepareDebugMessage(inputVector, i);
    MatchDescriptors(inputVector, i);

    cv::Mat debug_img{cv::Size(640, 480), CV_8UC3, cv::Scalar(0,0,0)};
    for (int j = nextStates[i]->methodIdxStart;
         j < nextStates[i]->methodIdxStop && j < (int)inputVector.size() &&
         j < (int)nextStates[i]->acceptableInput.size();
         j++) {
      double dist_j = GetDist(inputVector[j], nextStates[i]->acceptableInput[j]);
      tempDistance += dist_j;
      auto line_color = cv::Scalar(0, 0, 255);
      debug_msg += "[" + to_string(inputVector[j]) + "?=" +
                  to_string(nextStates[i]->acceptableInput[j]) + "]";
      if (dist_j <= nextStates[i]->threshold) {
        line_color = cv::Scalar(0, 255, 0);
        if ((j == methodIdxStop - 1) && (tempDistance < bestDistance)) {
          chosenState = i;
          bestDistance = tempDistance;
        }
      } else if (mistakes > 0) {
        BS_LOG(descriptor_path + " mistakes=" + std::to_string(mistakes), LogLevel::kKilo);
        mistakes--;
      } else {
        BS_LOG(descriptor_path + " REJECTED!", LogLevel::kKilo);
        accepted = false;
        break;
      }

      // Mark compared points
      if (visualize) VisualizeDraw(accepted, debug_img, inputVector, i, j, line_color);
    }
    if (visualize) VisualizeShow(accepted, debug_img, i);
    if (tempDistance < 0.001) BS_LOG(debug_msg, LogLevel::kKilo);
    else BS_LOG(debug_msg, LogLevel::kDebug);
  }
  // If possible move to that state, if not update idle counter.
  std::pair<bool, BehaviorState*> possible_state = 
    GoToState(std::pair<bool, int>{accepted, chosenState}, isMoving);
  if (possible_state.first) return possible_state.second;
  else return CheckIdleCounter();
}

void BehaviorState::MatchDescriptors(const std::vector<PointNorm>& inputVector, const int i) {
  const int idx_start = nextStates[i]->methodIdxStart;
  const int idx_stop = nextStates[i]->methodIdxStop;
  const int pt_count = idx_stop - idx_start;

  const PointNorm in_dim(inputVector[0].x, inputVector[0].y);
  const PointNorm descr_dim(nextStates[i]->acceptableInput[0].x, 
                            nextStates[i]->acceptableInput[0].y);

  std::vector<PointNorm> input; input.reserve(pt_count);
  for (int j = idx_start; j < idx_stop && j < (int)inputVector.size(); ++j) {
    input.push_back(inputVector[j]);
  }
  std::vector<PointNorm> descr; descr.reserve(pt_count);
  for (int j = idx_start; j < idx_stop; ++j) {
    descr.push_back(nextStates[i]->acceptableInput[j]);
  }

  cv::Mat debug_img{cv::Size(640, 480), CV_8UC3, cv::Scalar(0,0,0)};
  for (const auto& pt : descr) {
    auto nextSttPt = cv::Point(int(descr_dim.x * pt.x + 320), int(descr_dim.y * pt.y + 240));
    cv::circle(debug_img, nextSttPt, 3, cv::Scalar(0,255,0));
  }
  for (const auto& pt : input) {
    auto inputPt = cv::Point(int(in_dim.x * pt.x + 320), int(in_dim.y * pt.y + 240));
    cv::circle(debug_img, inputPt, 2, cv::Scalar(255,0,0));
  }
  cv::putText(debug_img, descriptor_path, cv::Point(10, 10), CV_FONT_VECTOR0, 0.4, cv::Scalar(255,255,255));
  cv::imshow("all points", debug_img);
  
  const size_t in_size = input.size();
  const size_t descr_size = descr.size();
  std::cout << "input.size() = " << in_size << std::endl;
  std::cout << "descr.size() = " << descr_size << std::endl;
  for (size_t n = 0; n < in_size || n < descr_size; ++n) {
    cv::Mat debug_img{cv::Size(640, 480), CV_8UC3, cv::Scalar(0,0,0)};
    double distance = 0.0;
    for (size_t m = 0; m < descr_size && m < in_size; ++m) {
      const size_t temp_idx = 
        (in_size > descr_size) ? (n + m) % in_size 
                               : (n + m) % descr_size;
      double dist = 
        (in_size > descr_size) ? GetDist(input[temp_idx], descr[m]) 
                               : GetDist(input[m], descr[temp_idx]);
      if (in_size > descr_size)
        std::cout << "|input[" << temp_idx << "] - descr[" << m << "]| = " << dist << std::endl;
      else
        std::cout << "|input[" << m << "] - descr[" << temp_idx << "]| = " << dist << std::endl;
      distance += dist;

      //----------------------------------------------------------------------
      cv::Point inputPt, nextSttPt;
      if (in_size > descr_size) {
        inputPt = cv::Point(int(in_dim.x * input[temp_idx].x + 320),
                            int(in_dim.y * input[temp_idx].y + 240));
        nextSttPt = cv::Point(int(descr_dim.x * descr[m].x + 320), 
                              int(descr_dim.y * descr[m].y + 240));
      } else {
        inputPt = cv::Point(int(in_dim.x * input[m].x + 320),
                            int(in_dim.y * input[m].y + 240));
        nextSttPt = cv::Point(int(descr_dim.x * descr[temp_idx].x + 320), 
                              int(descr_dim.y * descr[temp_idx].y + 240));
      }
      cv::circle(debug_img, inputPt, 2, cv::Scalar(255,0,0));
      cv::circle(debug_img, nextSttPt, 3, cv::Scalar(0,255,0));

      if (dist > nextStates[i]->threshold)
        cv::line(debug_img, inputPt, nextSttPt, cv::Scalar(0,0,255));
      else
        cv::line(debug_img, inputPt, nextSttPt, cv::Scalar(0,255,0));
      //----------------------------------------------------------------------

      
     // cv::imshow(descriptor_path + " matching", debug_img);
      //cv::waitKey();
    }
    std::cout << "Distance: " << distance << std::endl << std::endl;
    cv::putText(debug_img, descriptor_path + " Distance: " + std::to_string(distance),
                cv::Point(10, 10), CV_FONT_VECTOR0, 0.4, cv::Scalar(255,255,255));
    cv::imshow("matching", debug_img);
    //static int match_counter = 0;
    //cv::imwrite("matching_" + std::to_string(match_counter++) + ".png", debug_img);
    cv::waitKey();
  }

}

std::string BehaviorState::PrepareDebugMessage(
    const std::vector<PointNorm>& inputVector, const int i) {
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

void BehaviorState::VisualizeDraw(bool accepted, cv::Mat& debug_img,
                                  const std::vector<PointNorm>& inputVector,
                                  const int i, const int j,
                                  const cv::Scalar line_color) {
  auto inputPt = cv::Point((int)(inputVector[0].x * inputVector[j].x + 320), 
                           (int)(inputVector[0].y * inputVector[j].y + 240));
  auto nextSttPt = cv::Point((int)(nextStates[i]->acceptableInput[0].x * nextStates[i]->acceptableInput[j].x + 320), 
                             (int)(nextStates[i]->acceptableInput[0].y * nextStates[i]->acceptableInput[j].y + 240));
  cv::line(debug_img, inputPt, nextSttPt, line_color);
  cv::circle(debug_img, inputPt, 2, cv::Scalar(255,0,0));
  cv::circle(debug_img, nextSttPt, 3, cv::Scalar(0,255,0));
}

void BehaviorState::VisualizeShow(bool accepted, cv::Mat &debug_img, int i) {
  int wait_time = 0;
  auto color = cv::Scalar(255, 255, 255);
  if (accepted) {
    //wait_time = 1000;
    color = cv::Scalar(0, 255, 0);
  }
  cv::putText(debug_img, descriptor_path, cv::Point(15,15), CV_FONT_VECTOR0, 0.45, color);
  cv::imshow("StateCompare:"+std::to_string(i), debug_img);
  cv::waitKey(wait_time);
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

// BehaviorState* BehaviorState::ChangeState(vector<PointNorm> inputVector/*,
// vector<Rect> vRect*/)
//{
//	bool accepted = false;
//	int mistakes = 0;
//
//	for(int i=0; i<acceptableInput.size(); i++)
//	{
//		for(int j=1; j<acceptableInput[i].size(); j++)
//		{
//			if(GetDist(inputVector[j],acceptableInput[i][j]) >
// threshold)
//			{
//				if(mistakes>0)
//					mistakes--;
//				else
//				{
//					idleCounter++;
//					i = acceptableInput.size();
//					accepted = false;
//					break;
//				}
//			}
//			else
//			{
//				accepted = true;
//				//nextStates[i]->rectNumber = j;
//			}
//
//		}
//		if(accepted)
//		{
//			idleCounter = 0;
//			return this->nextStates[i];
//		}
//	}
//	if(idleCounter >= 100000)
//		return NULL;
//	else
//		return this;
//
//}

inline double BehaviorState::GetDist(PointNorm A, PointNorm B) {
  return sqrt((A.x - B.x) * (A.x - B.x) + (A.y - B.y) * (A.y - B.y));
}

void BehaviorState::SetNextStates(std::vector<BehaviorState*> nextStts) {
  nextStates = nextStts;
}

void BehaviorState::SetIdxStop(int idxStop) { this->methodIdxStop = idxStop; }

}  // namespace bd
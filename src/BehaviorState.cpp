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
  bool accepted = false;
  // Search best possible next state
  int chosenState = 0;
  double bestDistance = (double)INT_MAX;
  for (int i = 0; i < (int)nextStates.size(); i++) {
    double tempDistance = 0.0;
    int mistakes = GetAcceptedMissmatchCount(i);
    double weighted_threshold =
        (nextStates[i]->methodIdxStop - nextStates[i]->methodIdxStart) *
        nextStates[i]->threshold;
    
    std::string msg = "nextStates[" + std::to_string(i) +
                      "]:" + to_string(nextStates[i]->acceptableInput);
    BS_LOG(msg, LogLevel::kDebug);
    msg = "inputVector[" + std::to_string(i) + "]:" + to_string(inputVector);
    BS_LOG(msg, LogLevel::kDebug);
    std::string debug_msg =
        "States being compared[" + std::to_string(i) + "]: ";

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
      if ((dist_j <= nextStates[i]->threshold)
          /*|| (nextStates[i]->methodIdxStart > 11 && tempDistance <= weighted_threshold)*/) {
        // pt_match_count++;
        accepted = true;
        line_color = cv::Scalar(0, 255, 0);
        if ((j == methodIdxStop - 1) && (tempDistance < bestDistance)) {
          chosenState = i;
          bestDistance = tempDistance;
        }
      } else if (mistakes > 0) {
        mistakes--;
      } else {
        accepted = false;
        break;
      }

      // Mark compared points
      if (visualize) {
        auto inputPt = cv::Point((int)(inputVector[0].x * inputVector[j].x + 320), 
                                 (int)(inputVector[0].y * inputVector[j].y + 240));
        auto nextSttPt = cv::Point((int)(nextStates[i]->acceptableInput[0].x * nextStates[i]->acceptableInput[j].x + 320), 
                                   (int)(nextStates[i]->acceptableInput[0].y * nextStates[i]->acceptableInput[j].y + 240));
        cv::line(debug_img, inputPt, nextSttPt, line_color);
        cv::circle(debug_img, inputPt, 2, cv::Scalar(255,0,0));
        cv::circle(debug_img, nextSttPt, 3, cv::Scalar(0,255,0));
      }
    }
    if (visualize) Visualize(accepted, debug_img, i);
    if (tempDistance < 0.001) BS_LOG(debug_msg, LogLevel::kKilo);
    else BS_LOG(debug_msg, LogLevel::kDebug);
  }
  // If possible move to that state
  if (accepted) {
    if (nextStates[chosenState]->lastState == true && behType == 3) {
      if (!isMoving) {
        if (faintCounter <= 1) {
          idleCounter = 0;
          return this->nextStates[chosenState];
        } else {
          faintCounter--;
        }
      }
    } else {
      idleCounter = 0;
      return this->nextStates[chosenState];
    }
  }
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

void BehaviorState::Visualize(bool accepted, cv::Mat &debug_img, int i) {
  int wait_time = 1;
  auto color = cv::Scalar(255, 255, 255);
  if (accepted) {
    wait_time = 2000;
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
#pragma once
// STL includes
#include <fstream>
#include <strstream>
// OpenCV includes
#include <opencv\cv.h>
#include <opencv2\nonfree\nonfree.hpp>
// Project includes
#include "StructuresSM.h"

namespace bd {

class BehaviorState {
 private:
  std::vector<PointNorm> acceptableInput;
  std::vector<BehaviorState *> nextStates;

  double threshold;
  int idleCounter;
  int faintCounter;
  bool statsON;
  int methodIdxStart;
  int methodIdxStop;

  /*Function used within BehaviorFilter->Save() to count all connections present
   * in current behavior filter. (Recursive)*/
  int GetConnectionsNumber();
  /*Function that fills connections map.*/
  void MarkConnections(bool *map, int mapSize);
  /*Function used within BehaviorFilter->Save() to save each consequent
   * descriptor.*/
  void SaveDescriptor(std::ostream &output);
  /*Function used to collect statistics about current state points
   * constellation.*/
  void SaveStatistic(std::vector<PointNorm> inputVector);

 public:
  bool lastState;
  int sttNumber;
  int behType;
  std::string stateDescription;

  /*Arguments: threshold, chosen method for picking characteristic points,
   * number of points in method 2*/
  BehaviorState(double thresh = 0.2, int method = 0, int lPkt = 10);
  /*Arguments: acceptableInput, bool that states if it is a last
   * state,threshold, chosen method for picking characteristic points, number of
   * points in method 2*/
  BehaviorState(std::vector<PointNorm> accInput, bool isLast,
                double thresh = 0.2, int method = 0, int lPkt = 10);
  /*Arguments: acceptableInput, vector with all next states,bool that states if
   * it is a last state,threshold, chosen method for picking characteristic
   * points, number of points in method 2*/
  BehaviorState(std::vector<PointNorm> accInput,
                std::vector<BehaviorState *> nextstts, bool isLast,
                double thresh = 0.2, int method = 0, int lPkt = 10);
  /*Arguments: acceptableInput, vector with all next states,bool that states if
   * it is a last state, string with state description,threshold, chosen method
   * for picking characteristic points, number of points in method 2*/
  BehaviorState(std::vector<PointNorm> accInput,
                std::vector<BehaviorState *> nextstts, bool isLast,
                std::string sttDescr, double thresh = 0.2, int method = 0,
                int lPkt = 10);
  ~BehaviorState(void);

  /*Default function to move between states.*/
  BehaviorState *ChangeState(std::vector<PointNorm> inputVector);
  /*Function to move between states. Used for example when checking for fainted
   * people.*/
  BehaviorState *ChangeState(std::vector<PointNorm> inputVector, bool isMoving);

  /*Function to calculate distance of Points in input vectors.*/
  inline double GetDist(PointNorm A, PointNorm B);

  /*Sets next states vector for this Behavior State.*/
  void SetNextStates(std::vector<BehaviorState *> nextStts);
  /*Set vector of acceptable input for all connected states.*/
  void SetAcceptableInput(std::vector<std::vector<PointNorm>> accInput);
  /**/
  void SetDescription(std::string desc);
  /*Function used to set first states's stop index.*/
  void SetIdxStop(int idxStop);

  friend class BehaviorFilter;
  friend class DataBase;
};
}  // namespace bd
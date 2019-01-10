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

struct DistField {
  size_t n; //Index in input vector
  size_t m; //Index in descriptor vector
  double dist; //Distance between both points
  bool picked = false; //Marks if given pair was picked already
};
using VecDistField = std::vector<DistField>;
using DistFieldMatrix = std::vector<VecDistField>;

struct MatchDistance {
  double distance = (double)INT_MAX;
  size_t unassigned = 0;
  size_t assigned = 0;
};

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
  void Visualize(cv::Mat &debug_img, const VecDistField &result, const int i,
                 const PointNorm &in_dim, const PointNorm &descr_dim,
                 const double distance, const std::vector<PointNorm> &input,
                 const std::vector<PointNorm> &descr);
  int GetAcceptedMissmatchCount(int i);
  MatchDistance MatchDescriptors(const std::vector<PointNorm> &inputVector, const int i);
  /* Create distance field matrix M[n][m] that is sorted in two steps:
    a) each row n is sorted in ascending order of M[const][m - swapped]
    b) then, rows are sorted in ascending order of M[n - swapped][0]
    Example: 
    |2 4 3 12| a)  |2 3 4 12| b)  |1 5 6 11|
    |5 6 1 11| ==> |1 5 6 11| ==> |2 3 4 12|
    |8 7 9 10|     |7 8 9 10|     |7 8 9 10|
  */
  DistFieldMatrix CreateDistanceFieldMatrix(const std::vector<PointNorm> &input, 
                                            const std::vector<PointNorm> &descr);
  /* Go over distance field matrix to create a list of pairs input_pt => descr_pt.
     If input.size() != descr.size() some points will be left unassigned and will
     add to overall distance.
  */
  VecDistField CreatePairs(DistFieldMatrix &dist_matrix);
  BehaviorState* CheckIdleCounter();
  std::pair<bool, BehaviorState *> GoToState(std::pair<bool, int> next_state,
                                             bool is_moving);
  std::string PrepareDebugMessage(const std::vector<PointNorm> &inputVector,
                                  const int i);

 public:
  bool visualize;
  bool show_debug_msgs;
  std::string descriptor_path;
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
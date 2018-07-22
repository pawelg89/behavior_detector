#pragma once
#include <opencv\cv.h>
#include <opencv2\nonfree\nonfree.hpp>
#include "BehaviorState.h"
#include "StructuresSM.h"

using namespace std;
using namespace cv;
extern int log_level;

class BehaviorFilter
{
protected:
	/*Don't touch this!!*/
	vector<BehaviorState*> temp_StateHandles;
private:
	BehaviorState* firstState;
	BehaviorState* currentState;
	vector<int> method;
	int lPkt;
public:
	string behaviorDescription;
	int behaviorType;
	bool found;
	int objNumber;

	BehaviorFilter(void);
	BehaviorFilter(char* path);
	BehaviorFilter(char* path,int gs);
	~BehaviorFilter(void);

	/*Default function to check behavior on current filter.*/
	void Check(vector<PointNorm> input);
	/*Function to check behavior used if current filter is searching for fainting people.*/
	void Check(vector<PointNorm> input, bool isMoving);

	vector<int> BehaviorType();
	int GetCurrentStateNumber();
	void Rewrite(char* path, double thresholdNew);
	void SetFirstState(BehaviorState* first);
	void SetThreshold(double thresh);
	void SetBehaviorType(int type, string message);
	void Save(char* path);
	BehaviorState* GetFirstState();
	/*Function that lets you join another behavior filter as an alternate route.*/
	void Append(BehaviorFilter* appFilter);

	friend class DataBase;
};

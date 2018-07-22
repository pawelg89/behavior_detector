#pragma once
#include <vector>
#include <opencv\cv.h>
#include "StructuresSM.h"

using namespace std;
using namespace cv;

class BehaviorDescription
{
public:
	vector<vector<PointNorm>> descriptor;
	vector<int> v_sizes;
	int* sizes;
	bool* map;

	BehaviorDescription(void);
	~BehaviorDescription(void);

	void SaveBehaviorDescriptor(char* descriptor_name/*,int method = 0*/);
};


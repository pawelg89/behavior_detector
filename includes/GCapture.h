#pragma once
#include <vector>
#include "..\..\stdafx.h"
#include <opencv\cv.h>
#include <opencv2\core\core.hpp>
#include <opencv2\highgui\highgui.hpp>
#include <opencv2\video\background_segm.hpp>

using namespace std;
using namespace cv;
class GCapture
{
private:
	vector<string> pathDir;
	vector<HANDLE> watkiKamer;

public:
	void QueryFrame(Mat &frame, Mat &fore, int i);

	GCapture(void);
	GCapture(int cams_number);
	~GCapture(void);
};


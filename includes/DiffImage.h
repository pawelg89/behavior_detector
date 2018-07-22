#pragma once
#include <opencv\cv.h>
#include <opencv2\highgui\highgui.hpp>
#include <string.h>
#include "Convex.h"
#include "marked_frame.h"
using namespace std;
using namespace cv;
class DiffImage
{
public:
	CvMat*	new_objects;
	int		iterations;
	char*	file_name;
	marked_frame* MFrame;
	DiffImage(char* file);
	~DiffImage(void);
	/*Function which dynamically sets ROI to the frame. No more used.*/
	void GetROI(IplImage* frame, IplImage* frame2);
	/*Start point of application, using file from disk.*/
	void DiffImageAction();
	/*Start point of application, using IP camera.*/
	void DiffImageAction2();
	/*Not used, under development*/
	void CalcLibraryWeights(const char* Library_path, const char* Mask_path);
	void TrackObjects3D(Mat Frame, vector<Convex*> blob_vec, vector<vector<Point2f>>& marker_coord, int view);	
	void RestrictedAreaCheck(IplImage* frame, vector<Point2f> &vec,int camera, vector<Mat>H_f,vector<vector<Point2f>>& marker_coord,vector<Convex*> blob_vec);
};


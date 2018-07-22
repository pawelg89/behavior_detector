#pragma once
#include <opencv\cv.h>
#include <opencv2\nonfree\nonfree.hpp>
#include <list>
#include <fstream>
#include "detected_object.h"
#include "Helper.h"
#include "BehaviorFilter.h"
#include "BehaviorDescription.h"


using namespace std;
using namespace cv;

extern int log_level;

//======================================================================================
template <class type> bool load_data(string data_file, string data_name, type &data)
{	
	bool success = false;
	std::string temp_string = "";
	ifstream input;
	input.open(data_file);
	if(!input.is_open())
		return success;

	while(!input.eof())
	{
		input >> temp_string;
		if(temp_string == data_name)
		{
			input >> data;
			success = true;
			break;
		}
		else input >> temp_string;
	}
	input.close();

	return success;
}
//======================================================================================

class Convex
{
private:	
	const double A;
	const double AREA_TRESH;
	const double AH_TRESH;
	const double SALIENCE_TRESH;
	int global_counter;
	int waitTime;

	int method;
	int method0, method1, method2, method3;
	int lPkt;
	ofstream logFile;

	//Klasa pomocnicza przeliczajaca wspolrzedne
	Helper* GHelp;


	/**/
	void ConnectBodyFragments();
	/*Function used by ConnectBodyFragments.*/
	//BodyFragment* CBF(BodyFragment* first);

	vector<vector<Point>> contours;
	vector<Vec4i> hierarchy;
	//Vector containing subvectors with indices of NCM points for each contour
	vector<vector<int> >NCMs;
	//Vector containing each contour area
	vector<double> contour_areas;

	//vector with center points for each hull
	vector<Point> centers;
	//Automaton for behavior filtering
	BehaviorFilter* BF;


	void ClearVectors();

	/*Calculates the distance between Point C and a line segment containing points A and B.*/
	double Geth(cv::Point A, cv::Point B, cv::Point C);

	inline double GetDist(cv::Point A, cv::Point B);
	
	cv::Point ChangeCoords(cv::Point ox, cv::Point p);

	double GetAlfa(cv::Point p1, cv::Point prePt);

	double GetTheta(double x, double y, double alfa);

	/*Visualize contour parts that build up into a human*/
	void ShowContourElements(int rows, int cols);
	
	bool is_close_to(int master_index, int slave_index) ;
public:
	int morph_size;

	Convex(void);
	Convex(int frameWidth, int frameHeight, int camID);
	~Convex(void);


	/*Silhuettes of Human Images Extraction and Localization Detector*/
	void SHIELD(cv::Mat frame, cv::Mat frame_gray, int view);
	/*Silhuettes of Human Images Extraction and Localization Detector*/
	void SHIELD(cv::Mat frame, cv::Mat frame_gray,bool creatingDescriptors);
	/*Function that creates a convex hull on a given silhuette*/
	void NCM(cv::Mat frame, cv::Mat frame_gray);

	
	/*Function that allows second iteration of finding NCMs, by updating convex hull*/
	int UpdateHull(int startHull, int endHull, std::vector<cv::Point> &hull, std::vector<cv::Point> &contour,int NCMidx);

	/*Function that prepares input vectors for each object on frame.*/
	void BehaviorInput(Mat frame, vector<vector<Point>>hulls);
	/**/
	void BehaviorFiltersCheck(Mat frame/*, vector<vector<Point>>hulls*/);

	//------------------Zmienne i funkcje wykorzystywane podczas sledzenia obiektow-----------------------
	std::vector<cv::Point> temp_pos;
	std::vector<cv::Rect> temp_rect;
	std::vector<detected_object*>detected_objects;
	/*Input vectors for each object on frame.*/
	vector<vector<PointNorm>> behDescr;
	BehaviorDescription *BD;

	int track_objects(IplImage* frame, int c);
	/*Background update if scene changes too much*/
	bool is_background_ok(int width, int height);
};


#include "..\includes\Helper.h"
#define _USE_MATH_DEFINES
#include <math.h>
#include <iostream>

using namespace std;

Helper::Helper(void)
{
}


Helper::~Helper(void)
{
}

Helper::Helper(double wi, double he, double fo,double Ha, double alfaa,int came_w, int came_h)
{
	alfa = (90-alfaa)*M_PI/180;
	cam_w = came_w;
	cam_h = came_h;
	dx = wi/cam_w;
	dy = he/cam_h;
	f = fo;
	H = Ha;
}

Helper::Helper(double dxx, double dyy, double fo,double Ha, double alfaa,int came_w, int came_h, bool test)
{
	alfa = (90-alfaa)*M_PI/180;
	cam_w = came_w;
	cam_h = came_h;
	dx = dxx;
	dy = dyy;
	f = fo;
	H = Ha;
}

void Helper::CalculatePosition(int x, int y)
{
	double del_x = (x - cam_w/2)*dx;
	double del_y = (cam_h/2 - y)*dy;

	double beta = atan((del_y)/f);
	
	Z = H*tan(alfa + beta);
	if(alfa + beta >= M_PI/2)
	{
		cout<<alfa + beta<<endl;
	}

	X = sqrt(Z*Z + H*H)*((del_x)/sqrt(f*f + del_y*del_y));
}

void Helper::CalculateHeight(int x, int y)
{
	double del_x = (x - cam_w/2)*dx;
	double del_y = (cam_h/2 - y)*dy;

	double beta = atan((del_y)/f);
	
	double Z2 = H*tan(alfa + beta);

	obj_h = H*(1 - Z/Z2);
}

int Helper::GetImgWidth()
{
	return cam_w;
}

int Helper::GetImgHeight()
{
	return cam_h;
}

double Helper::Getdx()
{
	return dx;
}

double Helper::Getdy()
{
	return dy;
}

double Helper::GetAlpha()
{
	return alfa;
}

double Helper::GetFocal()
{
	return f;
}

double Helper::GetCamHeight()
{
	return H;
}

void Helper::Set(double dxx, double dyy, double fo, double Ha, double alfaa, int camW, int camH)
{
	alfa = alfaa;
	dx = dxx;
	dy = dyy;
	f = fo;
	H = Ha;
	cam_w = camW;
	cam_h = camH;
}
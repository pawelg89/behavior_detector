// STL includes
#define _USE_MATH_DEFINES
#include <math.h>
// OpenCV includes
#include <opencv\cv.h>
#include <opencv\cxcore.h>
#include <opencv2\highgui\highgui.hpp>
#include <opencv2\imgproc\imgproc.hpp>
// Project includes
#include "Convex.h"
#include "detected_object.h"

//-----------------------------------------------------------------------------
//---------- N I E -- D O T Y K A Æ ! -- G R O Z I -- Œ M I E RC I ¥ ----------
//-----------------------------------------------------------------------------
namespace bd {
using namespace std;
using namespace cv;

Point Convex::ChangeCoords(Point ox, Point p) {
  return Point(ox.y - p.y, ox.x - p.x);
}

double Convex::GetAlfa(Point p1, Point prePt) {
  double dx = (p1.x - prePt.x);
  double dy = (p1.y - prePt.y);

  if (dx > 0 && dy > 0)
    return atan(abs(dx / dy));
  else if (dx > 0 && dy < 0)
    return M_PI - atan(abs(dx / dy));
  else if (dx > 0 && dy == 0)
    return M_PI / 2;
  else if (dx < 0 && dy > 0)
    return 2 * M_PI - atan(abs(dx / dy));
  else if (dx < 0 && dy == 0)
    return 3 * M_PI / 2;
  else if (dx <= 0 && dy < 0)
    return M_PI + atan(abs(dx / dy));
  else if (dx == 0 && dy >= 0)
    return 0;
  else
    throw std::runtime_error("I did not expect this!");
}

double Convex::GetTheta(double x, double y, double alfa) {
  double theta = 0;
  if (x > 0 && y > 0)
    theta = atan(abs(y / x));
  else if (x > 0 && y < 0)
    theta = 2 * M_PI - atan(abs(y / x));
  else if (x < 0 && y >= 0)
    theta = M_PI - atan(abs(y / x));
  else if (x < 0 && y < 0)
    theta = atan(abs(y / x)) + M_PI;
  else if (x == 0 && y > 0)
    theta = M_PI / 2;
  else if (x == 0 && y < 0)
    theta = 3 * M_PI / 2;
  else if (x == 0 && y == 0)
    theta = 0;

  theta -= alfa;

  while (theta <= 0) theta += M_PI * 2;

  return theta;
}

int Convex::UpdateHull(int startHull, int endHull, vector<Point> &hull,
                       vector<Point> &contour, int NCMidx) {
  vector<Point> temp_hull;
  int nbr_ptsAdded = 0;
  bool correct = true;

  //------------------------EXTRACT SUBVECTOR OF
  //CONTOUR----------------------------------------
  int idxStart = 0;
  for (idxStart; idxStart < (int)contour.size(); idxStart++)
    if (contour[idxStart] == hull[startHull]) break;

  int idxStop = 0;
  for (idxStop; idxStop < (int)contour.size(); idxStop++)
    if (contour[idxStop] == hull[endHull]) break;

  vector<Point> subVec;
  if (idxStart < idxStop)
    for (int i = idxStart; i <= idxStop; i++) subVec.push_back(contour[i]);
  else
    for (int i = idxStart; i <= static_cast<int>(idxStop + contour.size()); i++)
      subVec.push_back(contour[i % contour.size()]);

  Point start = subVec[0];
  Point end = subVec[subVec.size() - 1];

  //------------------------------Debug----------------------------------------
  // cv::Mat drawing = Mat::zeros( 576,720, CV_8UC3 );
  // for(int i=0; i<contour.size();i++)
  //	circle(drawing,contour[i],0,cvScalar(180,180,180),1);
  // for(int i=startHull; i<= endHull+nbr_ptsAdded;i++)
  //	circle(drawing,hull[i],0,cvScalar(0,255,255),1);

  // circle(drawing,subVec[0],0,cvScalar(255,0,0),1);//blue start
  // circle(drawing, subVec[subVec.size() - 1],0,cvScalar(0,255,0),1);//green
  // stop  flip(drawing,drawing,0);  imshow("drawing",drawing);

  // cvWaitKey(0);
  //------------------------------------------------------------------------------------------

  // Point end2 = subVec[subVec.size() - 1];
  //------------------------FIND NCM IN CURRENT
  //SUBVECTOR---------------------------------------
  int ncmidx_subv = -1;
  for (int i = 0; i < (int)subVec.size(); i++)
    if (subVec[i] == contour[NCMidx]) ncmidx_subv = i;
  //------------------------CALCULATE HULL POINTS UNTIL
  //NCM------------------------------------
  Point prePt;
  Point p1;
  Point p2 = start;
  if (idxStart > 0) {
    prePt = contour[idxStart - 1];
    p1 = contour[idxStart - 1];
  } else  // Przypadek, gdy idxStart == 0
  {
    prePt = contour[contour.size() - 1];
    p1 = contour[contour.size() - 1];
  }
  do {
    prePt = p1;
    p1 = p2;
    double alfa = GetAlfa(p1, prePt);
    double temp_theta = 1000000;
    Point temp_p2;
    int it = 0;
    // Przesun iterator na aktualny punkt, zeby nastepny for wyszukiwal tylko
    // wsrod punktow  znajdujacych sie za p1 ale przed NCM'em
    for (it; it <= ncmidx_subv; it++)
      if (subVec[it] == p1) {
        it++;
        break;
      }
    for (it; it <= ncmidx_subv; it++) {
      temp_p2 = ChangeCoords(p1, subVec[it]);
      double theta = GetTheta((double)temp_p2.x, (double)temp_p2.y, alfa);
      if (theta <= temp_theta) {
        temp_theta = theta;
        p2 = subVec[it];
      }
    }
    // int ithull = 0;
    // for(ithull;hull[ithull] != end; ithull++);
    temp_hull.insert(temp_hull.end(), p2);
    nbr_ptsAdded++;
    if (nbr_ptsAdded > static_cast<int>(subVec.size())) {
      correct = false;
      break;
    }
  } while (p2 != subVec[ncmidx_subv]);
  //--------------------CALCULATE HULL POINTS FROM NCM TO
  //HULL_STOP-----------------------------
  prePt = subVec[ncmidx_subv - 1];
  p1 = subVec[ncmidx_subv - 1];
  p2 = subVec[ncmidx_subv];
  do {
    prePt = p1;
    p1 = p2;
    double alfa = GetAlfa(p1, prePt);
    double temp_theta = 10000000;
    Point temp_p2;
    int it = ncmidx_subv;
    // Przesun iterator na aktualny punkt p1, tak zeby p2 byl wyszukiwany
    // wsrod punktow znajdujacych sie za pkt p1
    for (it; it < static_cast<int>(subVec.size()); it++)
      if (subVec[it] == p1) {
        it++;
        break;
      }
    for (it; it < static_cast<int>(subVec.size()); it++) {
      temp_p2 = ChangeCoords(p1, subVec[it]);
      double theta = GetTheta((double)temp_p2.x, (double)temp_p2.y, alfa);
      if (theta <= temp_theta) {
        temp_theta = theta;
        p2 = subVec[it];
      }
    }
    /*int ithull = 0;
    for(ithull; hull[ithull] != end; ithull++);*/
    temp_hull.insert(temp_hull.end(), p2);
    nbr_ptsAdded++;
    if (nbr_ptsAdded > static_cast<int>(subVec.size()) || !correct) {
      correct = false;
      break;
    }
  } while (p2 != end);
  //------------LAST ADDED POINT WAS SAME AS 'end' SO WE NEED TO REMOVE
  //IT--------------------

  for (int ithull = 0;
       correct && ithull < static_cast<int>(temp_hull.size()) - 1; ithull++) {
    if (temp_hull[ithull] == temp_hull[ithull + 1]) {
      temp_hull.erase(temp_hull.begin() + ithull);
      nbr_ptsAdded--;
    }
  }

  if (correct)
    hull.insert(hull.begin() + endHull, temp_hull.begin(), temp_hull.end());
  else
    nbr_ptsAdded = 0;

  temp_hull.clear();

  // cv::Mat drawing = Mat::zeros( 576,720, CV_8UC3 );
  // for(int i=0; i<contour.size();i++)
  //	circle(drawing,contour[i],0,cvScalar(180,180,180),1);
  // for(int i=startHull; i<= endHull+nbr_ptsAdded;i++)
  //	circle(drawing,hull[i],0,cvScalar(0,255,255),1);

  // circle(drawing,subVec[0],0,cvScalar(255,0,0),1);//blue start
  // circle(drawing, subVec[subVec.size() - 1],0,cvScalar(0,255,0),1);//green
  // stop  flip(drawing,drawing,0);  imshow("drawing",drawing);

  //	cvWaitKey(0);

  return nbr_ptsAdded;
}
}  // namespace bd
#include "..\..\behavior_detector\includes\Convex.h"
// STL includes
#define _USE_MATH_DEFINES
#include <math.h>
// OpenCV includes
#include <opencv\cv.h>
#include <opencv\cxcore.h>
#include <opencv2\highgui\highgui.hpp>
#include <opencv2\imgproc\imgproc.hpp>
// Project includes
#include "..\..\stdafx.h"
#include "..\includes\BehaviorFilter.h"
#include "..\includes\UpdateHull.h"
#include "..\includes\collector.h"
#include "..\includes\detected_object.h"
#include "..\includes\logger.h"
#include "..\includes\signaler.h"
#include "..\includes\timer.h"

static int FrameCounter = 0;
//=============================================================================
//=============================OBSLUGA MYSZY===================================
CvPoint pt_clicked;  // wskaznik na obiekt przechowujacy aktualnie wybrany punkt
// funkcja obslugi myszy
void MouseEvent2(int ev, int x, int y, int flags, void *param) {
  switch (ev) {
    // odczyt punktu z obrazu po wcisnieciu lewego przycisku myszy
    case CV_EVENT_LBUTTONDOWN:
      pt_clicked.x = x;
      pt_clicked.y = y;
      break;
    default:
      break;
  }
}
void ClearPoint2(CvPoint p) {
  p.x = -1;
  p.y = -1;
}
void SetPoint(int x, int y) {
  pt_clicked.x = x;
  pt_clicked.y = y;
}

namespace bd {

using namespace std;
using namespace cv;

inline int Convex_LOG(std::string msg, LogLevel level, bool new_line = true) {
  return LOG("Convex", msg, level, new_line);
}

Convex::Convex(void)
    : A(5.0),
      AH_TRESH(10.0),
      AREA_TRESH(0.01),
      SALIENCE_TRESH(1.3),
      global_counter_(0),
      wait_time_(1),
      clears_counter_(0) {
  char tmp_buff[100] = "";
  morph_size = 2;

  ostrstream zapis(tmp_buff, (int)sizeof(tmp_buff), ios::app);
  if (!load_data("parameters.txt", "method", method_)) method_ = 0;
  zapis.seekp(0);
  zapis << "chosen method: " << method_;
  Convex_LOG(tmp_buff, LogLevel::kSetup);

  if (!load_data("parameters.txt", "lPkt", l_pkt_)) l_pkt_ = 10;
  zapis.seekp(0);
  zapis << "Number of points: " << l_pkt_;
  Convex_LOG(tmp_buff, LogLevel::kSetup);

  if (!load_data("parameters.txt", "method0", method0_)) method0_ = 1;
  if (!load_data("parameters.txt", "method1", method1_)) method1_ = 1;
  if (!load_data("parameters.txt", "method2", method2_)) method2_ = 1;
  if (!load_data("parameters.txt", "method3", method3_)) method3_ = 1;
  // Ustawienia dla sekwencji Parking_1
  GHelp = new Helper(0.00523, 0.00294, 0.0045, 5.91, 16.1, 1440, 1080);

  BD = new BehaviorDescription();
  this->ClearVectors();
}

Convex::Convex(int frameWidth, int frameHeight, int camID)
    : A(5.0),
      AH_TRESH(10.0),
      AREA_TRESH(0.01),
      SALIENCE_TRESH(1.3),
      global_counter_(0),
      wait_time_(1),
      clears_counter_(0) {
  char tmp_buff[100] = "";
  morph_size = 2;

  ostrstream zapis(tmp_buff, (int)sizeof(tmp_buff), ios::app);
  if (!load_data("parameters.txt", "method", method_)) method_ = 0;
  zapis.seekp(0);
  zapis << "chosen method: " << method_;
  Convex_LOG(tmp_buff, LogLevel::kSetup);

  if (!load_data("parameters.txt", "lPkt", l_pkt_)) l_pkt_ = 10;
  zapis.seekp(0);
  zapis << "Number of points: " << l_pkt_;
  Convex_LOG(tmp_buff, LogLevel::kSetup);

  if (!load_data("parameters.txt", "method0", method0_)) method0_ = 1;
  if (!load_data("parameters.txt", "method1", method1_)) method1_ = 1;
  if (!load_data("parameters.txt", "method2", method2_)) method2_ = 1;
  if (!load_data("parameters.txt", "method3", method3_)) method3_ = 1;

  GHelp = new Helper(0.00523, 0.00294, 0.0045, 5.91, 16.1, 1440, 1080);

  this->ClearVectors();
}

static int GloCTR = 0;
Convex::~Convex(void) {
  delete GHelp;
  ClearVectors();
  detected_objects.clear();
  Convex_LOG("Called: " + std::to_string(GloCTR), LogLevel::kSetup);
}

void Convex::SHIELD(Mat frame, Mat fore, int view) {
  std::string message = "";
  bd::Timer timer(false);
  auto collector = &Collector::getInstance();

  Convex_LOG("SHIELD(Mat frame, Mat fore, int view) called " +
                 std::to_string(GloCTR++),
             LogLevel::kMega);
  message += timer.PrintElapsed("Convex_log");
  collector->AddData("Convexlog", timer.last_elapsed);

  if (Signaler::getInstance().CheckAndReset("reset_tracker")) {
    detected_objects.clear();
    Convex_LOG(
        "detected_objects.clear(); call: " + std::to_string(++clears_counter_),
        LogLevel::kDetailed);
  }
  message += timer.PrintElapsed("SignalNClear", false);
  collector->AddData("SignalNClear", timer.last_elapsed);

  this->NCM(frame, fore);
  message += timer.PrintElapsed("NCM", false);
  collector->AddData("NCM", timer.last_elapsed);

  this->track_objects(new IplImage(frame), view);
  message += timer.PrintElapsed("track_objects", false);
  collector->AddData("trackobjects", timer.last_elapsed);

  this->BehaviorFiltersCheck(frame);
  message += timer.PrintElapsed("BehaviorFiltersCheck", false);
  collector->AddData("BehaviorFiltersCheck", timer.last_elapsed);

  imshow("fore", fore);
  message += timer.PrintElapsed("imshow", false);
  collector->AddData("imshow", timer.last_elapsed);

  this->ClearVectors();
  timer.Stop();
  double elapsed = timer.Elapsed() * 1000;
  if (elapsed > 40.0) {
    Convex_LOG("Elapsed time[ms]: " + std::to_string(elapsed) + " " + message,
               LogLevel::kDefault);
  }
  collector->AddData("ConvexShield", elapsed);
}

void Convex::HandleMouseCallbacks(Mat frame) {
  // Obsluga dodawania punktow do deskryptora
  setMouseCallback("Create Descriptor Window", MouseEvent2);
  ClearPoint2(pt_clicked);

  imshow("Create Descriptor Window", frame);
  key_ = static_cast<char>(cvWaitKey(wait_time_)); 

  if (key_ == 'p' || key_ == 'P') {
    if (wait_time_ == 1)
      wait_time_ = 0;
    else
      wait_time_ = 1;
  }
}

void Convex::SHIELD(Mat frame, Mat fore, bool creatingDescriptors) {
  if (Signaler::getInstance().CheckAndReset("reset_tracker")) {
    if (!creatingDescriptors) key_ = 'e';
    detected_objects.clear();
    Convex_LOG(
        "detected_objects.clear(); call: " + std::to_string(++clears_counter_),
        LogLevel::kKilo);
  } else {
    key_ = ' ';
  }
  /* Save created descriptor if pressed 'e' */
  if (key_ == 'e' || key_ == 'E') SaveDescriptor();

  /* Calculate convex hulls and descriptors. todo: rename this function */
  this->NCM(frame, fore);

  /* Choose middle of contour by hand or simply pick middle of image */
  if (creatingDescriptors) HandleMouseCallbacks(frame);
  else SetPoint(frame.cols / 2, frame.rows / 2);
  /* Select clicked contour */
  SelectContour(frame);

  /* Clean up */
  this->ClearVectors();
  GloCTR++;
}

void Convex::SelectContour(Mat frame) {
  if (pt_clicked.x != -1) {
    std::vector<PointNorm> temp_descriptors;
    // finding nearest
    double max_dist = 10000000000;
    int nearest_index;
    for (size_t i = 0; i < this->centers.size(); i++) {
      if (this->GetDist(pt_clicked, this->centers[i]) < max_dist) {
        max_dist = this->GetDist(pt_clicked, this->centers[i]);
        nearest_index = i;
      }
    }
    /*Save image containing only selected object rect*/
    SaveContourROI(frame, nearest_index);

    for (size_t i = 0; i < this->behDescr[nearest_index].size(); i++) {
      temp_descriptors.push_back(this->behDescr[nearest_index][i]);
    }
    std::string msg = "temp_descriptors:" + to_string(temp_descriptors);
    Convex_LOG(msg, LogLevel::kCritical);
    this->BD->descriptor.push_back(temp_descriptors);
    this->BD->v_sizes.push_back((int)temp_descriptors.size());
  }
}

void Convex::SaveContourROI(Mat frame, int nearest_index) {
  frame.adjustROI(
      -temp_rect[nearest_index].y,
      (temp_rect[nearest_index].y + temp_rect[nearest_index].height) -
          frame.rows,
      -temp_rect[nearest_index].x,
      (temp_rect[nearest_index].x + temp_rect[nearest_index].width) -
          frame.cols);
  char fileName_buff[100];
  sprintf(fileName_buff, "descr/%d.bmp", GloCTR);
  imshow("cut", frame);
  cvWaitKey(10);
  imwrite(fileName_buff, frame);
}

void Convex::SaveDescriptor() {
  BD->sizes = new int[BD->descriptor.size()];
  for (size_t i = 0; i < BD->descriptor.size(); i++)
    BD->sizes[i] = BD->v_sizes[i];

  BD->SaveBehaviorDescriptor();
  BD->Clear();
}

void Convex::ClearVectors() {
  contours.clear();
  hulls.clear();
  NCMs.clear();
  contour_areas.clear();

  temp_pos.clear();
  temp_rect.clear();
  behDescr.clear();
}

void Convex::NCM(Mat frame, Mat frame_gray, bool visualize) {
  blur(frame_gray, frame_gray, Size(3, 3));
  //	Ujednolicenie obiektow
  Mat element =
      getStructuringElement(2, Size(2 * morph_size + 1, 2 * morph_size + 1),
                            Point(morph_size, morph_size));
  morphologyEx(frame_gray, frame_gray, MORPH_OPEN, element);
  element =
      getStructuringElement(2, Size(2 * morph_size + 5, 2 * morph_size + 5),
                            Point(morph_size, morph_size));
  morphologyEx(frame_gray, frame_gray, MORPH_CLOSE, element);

  // Find contours
  findContours(frame_gray.clone(), contours, hierarchy, CV_RETR_EXTERNAL,
               CV_CHAIN_APPROX_SIMPLE, Point(0, 0));

  // Usuniêcie ma³ych obiektów
  for (int i = 0; i < (int)contours.size(); i++) {
    if (contours[i].size() < 50) {
      contours.erase(contours.begin() + i);
      i--;
    }
  }
  if (contours.size() == 0) return;

  // Przygotowanie wektorow dla Kalman Filter
  temp_rect.resize(contours.size());
  temp_pos.resize(contours.size());
  centers.resize(contours.size());
  // Interpolacja
  for (int i = 0; i < (int)contours.size(); i++) {
    int rect_leftmost = INT_MAX;
    int rect_rightmost = INT_MIN;
    int rect_topmost = INT_MAX;
    int rect_bottomost = INT_MIN;

    for (int j = 0; j < (int)contours[i].size(); j++) {
      Point A = contours[i][j];
      if (rect_leftmost > A.x) rect_leftmost = A.x;
      if (rect_rightmost < A.x) rect_rightmost = A.x;
      if (rect_topmost > A.y) rect_topmost = A.y;
      if (rect_bottomost < A.y) rect_bottomost = A.y;

      Point B = contours[i][(j + 1) % contours[i].size()];
      if (GetDist(A, B) >= 2) {
        vector<Point>::iterator it;
        it = contours[i].begin();
        Point newPt = Point(static_cast<int>((A.x + B.x) / 2.0),
                            static_cast<int>((A.y + B.y) / 2.0));
        if (j == contours[i].size() - 1)
          it = contours[i].insert(contours[i].end(), newPt);
        else
          it = contours[i].insert(it + (j + 1) % contours[i].size(), newPt);
        j--;
      }
    }
    temp_rect[i] = Rect(rect_leftmost, rect_topmost,
                        std::abs(rect_leftmost - rect_rightmost),
                        std::abs(rect_topmost - rect_bottomost));
    temp_pos[i] = Point(temp_rect[i].x + temp_rect[i].width / 2,
                        temp_rect[i].y + temp_rect[i].height);
    centers[i] = Point((rect_rightmost + rect_leftmost) / 2,
                       (rect_bottomost + rect_topmost) / 2);
  }

  threshold(frame_gray, frame_gray, 1, 255, CV_THRESH_BINARY);

  //------------------------------------------------------------------------------------------------
  // Wyliczamy powierzchnie konturów
  contour_areas.resize(contours.size());
  for (size_t i = 0; i < contours.size(); i++)
    contour_areas[i] = contourArea(contours[i]);

  hulls.resize(contours.size());
  for (size_t i = 0; i < contours.size(); i++)
    convexHull(Mat(contours[i]), hulls[i], true);

  NCMs.resize(contours.size());
  int number_iterations = 2;
  for (int iter = 0; iter < number_iterations; iter++) {
    for (size_t i = 0; i < contours.size(); i++) {
      vector<int> ncm;
      int NCM_counter = 1;
      int previdx = hulls[i].size() - 1;
      Point prev = hulls[i][previdx];
      Point curr = hulls[i][previdx];
      for (size_t j = 0; j < hulls[i].size(); j++) {
        if (j > 0) previdx = j - 1;
        prev = curr;
        curr = hulls[i][j];

        // czyli j i j-1 element sa dostatecznie odlegle
        // a - dlugosc podstawy trojkata zbudowanego przez odcinek miedzy
        // punktami  curr i prev oraz odcinek prostopadly laczacy z
        // potencjalnymi ncm'ami
        double a = GetDist(prev, curr);
        if (a > A) {
          int start = -1;
          int stop = -1;
          for (size_t k = 0; k < contours[i].size(); k++) {
            if (contours[i][k] == prev) start = k;
            if (contours[i][k] == curr) stop = k;
            if (start != -1 && stop != -1) break;
          }

          double h = 0;
          int idx = 0;
          bool found = false;
          Point farthest_point = Point(-10, -10);
          // Vector z konturem wkleslosci, posluzy do polieczenia pola
          vector<Point> concave;
          // Jesli start > stop, tj poczatek konturu znajduje sie pomiedzy nimi,
          // to ten przypadek nie zostanie wyliczony. Found = false i dalej nic
          // sie nie stanie
          for (int k = start; k <= stop; k++) {
            concave.push_back(Point(contours[i][k]));
            double temp = Geth(curr, prev, contours[i][k]);
            if (h < temp) {
              h = temp;
              idx = k;
              farthest_point = contours[i][k];
              found = true;
            }
          }

          // Pole wkleslosci
          double concave_area = 0;
          if (concave.size() > 0) concave_area = contourArea(concave);

          if (found && (a / h <= AH_TRESH) &&
              concave_area / contour_areas[i] > AREA_TRESH) {
            Point nbr_pt = Point(farthest_point);
            nbr_pt.x += 3;
            nbr_pt.y -= 3;
            NCM_counter++;

            ncm.push_back(idx);

            if (number_iterations > 1) {
              // If we need more iterations, Update Hull with new Points
              // containing  NCM that we have just found
              int nbr_ptsAdded =
                  UpdateHull(previdx, j, hulls[i], contours[i], idx);
              j += nbr_ptsAdded;
            }
          }
        }
      }
      for (size_t ncm_iter = 0; ncm_iter < ncm.size(); ncm_iter++) {
        vector<int>::iterator it;
        it = NCMs[i].end();
        it = NCMs[i].insert(it, ncm[ncm_iter]);
      }
    }
  }
  BehaviorInput(frame, hulls);
  if (visualize) VisualizeNCM(frame);
}

void Convex::VisualizeNCM(Mat frame) {
  CvFont font;
  cvInitFont(&font, CV_FONT_VECTOR0, 0.5, 0.5, 0, 1);
  auto yellow = Scalar(0, 255, 255);
  auto green = Scalar(0, 255, 0);
  char pos_buff[100] = "";
  ostrstream zapis(pos_buff, (int)sizeof(pos_buff), ios::app);
  for (size_t i = 0; i < centers.size(); ++i) {
    zapis.seekp(0);
    zapis << "[" << centers[i].x << "," << centers[i].y << "]";
    putText(frame, pos_buff, temp_pos[i], font.font_face, 0.35, green);
  }
  for (size_t i = 0; i < contours.size(); i++) {
    drawContours(frame, contours, i, yellow, 1, 8, vector<Vec4i>(), 0, Point());
  }
  for (size_t i = 0; i < hulls.size(); ++i) {
    drawContours(frame, hulls, i, green, 1, 8, vector<Vec4i>(), 0, Point());
  }
}

double Convex::Geth(Point A, Point B, Point C) {
  // double h = 0.0;
  double a =
      sqrt((double)(A.x - B.x) * (A.x - B.x) + (A.y - B.y) * (A.y - B.y));
  double b =
      sqrt((double)(A.x - C.x) * (A.x - C.x) + (A.y - C.y) * (A.y - C.y));
  double c =
      sqrt((double)(B.x - C.x) * (B.x - C.x) + (B.y - C.y) * (B.y - C.y));
  double p = 0.5 * (a + b + c);
  double P = sqrt(p * (p - a) * (p - b) * (p - c));

  return 2 * P / a;
}

inline double Convex::GetDist(Point A, Point B) {
  return sqrt((double)(A.x - B.x) * (A.x - B.x) + (A.y - B.y) * (A.y - B.y));
}

double Rest(double x, double y) {
  int tempX = (int)x;
  int tempY = (int)y;

  return (double)(x / y - tempX / tempY);
}

void Convex::BehaviorInput(Mat frame, vector<vector<Point>> hulls) {
  std::string msg = "Contours size: " + std::to_string(this->contours.size());
  Convex_LOG(msg, LogLevel::kMega);

  if (method0_) {
    // Wektor z punktami charakterystycznymi potrzebny do wykrywania zachowañ,
    // wersja normalizowana przez wysokosc i szerokosc
    vector<vector<PointNorm>> points_picture;
    points_picture.resize((int)contours.size());

    for (size_t i = 0; i < hulls.size(); i++) {
      double temp_distA = 0;
      double temp_distB = 0;
      PointNorm temp_pointA = PointNorm(-1, -1);
      PointNorm temp_pointB = PointNorm(-1, -1);

      double temp_distC = 0;
      double temp_distD = 0;
      PointNorm temp_pointC = PointNorm(-1, -1);
      PointNorm temp_pointD = PointNorm(-1, -1);

      for (size_t j = 0; j < hulls[i].size(); j++) {
        if (hulls[i][j].x < centers[i].x) {
          if (hulls[i][j].y < centers[i].y) {
            if (GetDist(hulls[i][j], centers[i]) >= temp_distA) {
              temp_distA = GetDist(hulls[i][j], centers[i]);
              temp_pointA = PointNorm(
                  (double)(hulls[i][j].x - centers[i].x) / temp_rect[i].width,
                  (double)(hulls[i][j].y - centers[i].y) / temp_rect[i].height);
            }
          } else {
            if (GetDist(hulls[i][j], centers[i]) > temp_distC) {
              temp_distC = GetDist(hulls[i][j], centers[i]);
              temp_pointC = PointNorm(
                  (double)(hulls[i][j].x - centers[i].x) / temp_rect[i].width,
                  (double)(hulls[i][j].y - centers[i].y) / temp_rect[i].height);
            }
          }
        } else {
          if (hulls[i][j].y < centers[i].y) {
            if (GetDist(hulls[i][j], centers[i]) > temp_distB) {
              temp_distB = GetDist(hulls[i][j], centers[i]);
              temp_pointB = PointNorm(
                  (double)(hulls[i][j].x - centers[i].x) / temp_rect[i].width,
                  (double)(hulls[i][j].y - centers[i].y) / temp_rect[i].height);
            }
          } else {
            if (GetDist(hulls[i][j], centers[i]) > temp_distD) {
              temp_distD = GetDist(hulls[i][j], centers[i]);
              temp_pointD = PointNorm(
                  (double)(hulls[i][j].x - centers[i].x) / temp_rect[i].width,
                  (double)(hulls[i][j].y - centers[i].y) / temp_rect[i].height);
            }
          }
        }
      }
      points_picture[i].push_back(temp_pointA);
      points_picture[i].push_back(temp_pointB);
      points_picture[i].push_back(temp_pointC);
      points_picture[i].push_back(temp_pointD);
    }

    this->behDescr.resize(centers.size());
    for (size_t i = 0; i < centers.size(); i++) {
      behDescr[i].push_back(PointNorm(centers[i].x, centers[i].y));
      behDescr[i].insert(behDescr[i].end(), points_picture[i].begin(),
                         points_picture[i].end());
    }
  }
  if (method1_) {
    // Wektor z punktami charakterystycznymi potrzebny do wykrywania zachowañ,
    // wersja normalizowana przez wysokosc
    vector<vector<PointNorm>> points_picture;
    points_picture.resize((int)contours.size());

    for (size_t i = 0; i < hulls.size(); i++) {
      double temp_distA = 0;
      double temp_distB = 0;
      PointNorm temp_pointA = PointNorm(-1, -1);
      PointNorm temp_pointB = PointNorm(-1, -1);

      double temp_distC = 0;
      double temp_distD = 0;
      PointNorm temp_pointC = PointNorm(-1, -1);
      PointNorm temp_pointD = PointNorm(-1, -1);

      for (size_t j = 0; j < hulls[i].size(); j++) {
        if (hulls[i][j].x < centers[i].x) {
          if (hulls[i][j].y < centers[i].y) {
            if (GetDist(hulls[i][j], centers[i]) >= temp_distA) {
              temp_distA = GetDist(hulls[i][j], centers[i]);
              temp_pointA = PointNorm(
                  (double)(hulls[i][j].x - centers[i].x) / temp_rect[i].height,
                  (double)(hulls[i][j].y - centers[i].y) / temp_rect[i].height);
            }
          } else {
            if (GetDist(hulls[i][j], centers[i]) > temp_distC) {
              temp_distC = GetDist(hulls[i][j], centers[i]);
              temp_pointC = PointNorm(
                  (double)(hulls[i][j].x - centers[i].x) / temp_rect[i].height,
                  (double)(hulls[i][j].y - centers[i].y) / temp_rect[i].height);
            }
          }
        } else {
          if (hulls[i][j].y < centers[i].y) {
            if (GetDist(hulls[i][j], centers[i]) > temp_distB) {
              temp_distB = GetDist(hulls[i][j], centers[i]);
              temp_pointB = PointNorm(
                  (double)(hulls[i][j].x - centers[i].x) / temp_rect[i].height,
                  (double)(hulls[i][j].y - centers[i].y) / temp_rect[i].height);
            }
          } else {
            if (GetDist(hulls[i][j], centers[i]) > temp_distD) {
              temp_distD = GetDist(hulls[i][j], centers[i]);
              temp_pointD = PointNorm(
                  (double)(hulls[i][j].x - centers[i].x) / temp_rect[i].height,
                  (double)(hulls[i][j].y - centers[i].y) / temp_rect[i].height);
            }
          }
        }
      }
      points_picture[i].push_back(temp_pointA);
      points_picture[i].push_back(temp_pointB);
      points_picture[i].push_back(temp_pointC);
      points_picture[i].push_back(temp_pointD);
    }

    // Draw points of each person
    // for(int i=0; i<points_picture.size(); i++)
    //{
    //	circle(frame,centers[i],1,cvScalar(255,0,0),2);
    //}

    // vector<vector<PointNorm>> behDescr;
    // behDescr.resize(1);
    this->behDescr.resize(centers.size());
    for (size_t i = 0; i < centers.size(); i++) {
      behDescr[i].push_back(PointNorm(centers[i].x, centers[i].y));
      behDescr[i].insert(behDescr[i].end(), points_picture[i].begin(),
                         points_picture[i].end());
    }
  }

  if (method2_) {
    // N punktów konturu wybieranych równomiernie

    vector<vector<PointNorm>> points_picture;
    points_picture.resize((int)contours.size());

    for (int i = 0; i < (int)contours.size(); i++) {
      int lPktKonturu = (int)contours[i].size();
      // Na wypadek zbyt du¿ej iloœci punktów
      if (l_pkt_ > lPktKonturu) break;

      int step = lPktKonturu / l_pkt_;
      double buffer = 0;  // Rest((double)lPktKonturu,(double)lPkt);
      int stepCTR = 0;
      for (int j = 0; j < lPktKonturu && stepCTR < l_pkt_;
           j += (step + (int)buffer)) {
        PointNorm tempP;
        tempP.x = (double)((contours[i][j].x - centers[i].x) /
                           (double)temp_rect[i].height);
        tempP.y = (double)((contours[i][j].y - centers[i].y) /
                           (double)temp_rect[i].height);
        stepCTR++;
        points_picture[i].push_back(tempP);

        if (buffer < 1.0) {
          buffer += Rest((double)lPktKonturu, (double)l_pkt_);
        } else {
          buffer -= 1.0;
          buffer += Rest((double)lPktKonturu, (double)l_pkt_);
        }
      }
    }
    this->behDescr.resize(centers.size());
    for (size_t i = 0; i < centers.size(); i++) {
      behDescr[i].push_back(PointNorm(centers[i].x, centers[i].y));
      behDescr[i].insert(behDescr[i].end(), points_picture[i].begin(),
                         points_picture[i].end());
    }
  }
  if (method3_) {  // Wersja do sprawdzenia, czy dobrze jest obslugiwana
                   // wypuklosc ktora moze wyst¹piæ pomiêdzy koñcem i pocz¹tkiem
                   // ( zawiniêcie siê numeracji punktow w wektorze )
    vector<vector<PointNorm>> points_picture;
    int contours_size = (int)contours.size();
    points_picture.resize(contours_size);

    for (int i = 0; i < contours_size; i++) {
      // NCMs
      for (int j = 0; j < (int)NCMs[i].size(); j++) {
        PointNorm tempPt;
        tempPt.x = ((double)contours[i][NCMs[i][j]].x - centers[i].x) /
                   (double)temp_rect[i].height;
        tempPt.y = ((double)contours[i][NCMs[i][j]].y - centers[i].y) /
                   (double)temp_rect[i].height;

        points_picture[i].push_back(tempPt);
      }

      // AntiNCMs
      int idxLeft = -1;
      bool leftSet = false;
      int idxRight = -1;
      bool rightSet = false;
      for (int j = 0; j < (int)hulls[i].size(); j++) {
        Point left = hulls[i][j];
        Point right = hulls[i][(j + 1) % hulls[i].size()];
        double dist = GetDist(left, right);

        // Ustawienie prawego znacznika
        if ((dist > A) && leftSet && !rightSet) {
          idxRight = j;  //<-- Point left
          rightSet = true;
        }
        // Ustawienie lewego znacznika
        if ((dist > A) && !leftSet) {
          idxLeft = j + 1;  //<-- Point right
          leftSet = true;
        }
        // Gdy oba znaczniki s¹ ustawione, to wyszukujemy PCM pomiêdzy nimi
        if (leftSet && rightSet) {
          double H = 0.0;
          int idxH = -1;
          for (int k = idxLeft + 1; k < idxRight; k++) {
            double temph =
                Geth(hulls[i][idxLeft], hulls[i][idxRight], hulls[i][k]);
            if (temph > H) {
              H = temph;
              idxH = k;
            }
          }
          // Jesli znaleziono jakis punkt PCM
          if (idxH != -1) {
            points_picture[i].push_back(
                PointNorm(((double)hulls[i][idxH].x - centers[i].x) /
                              (double)temp_rect[i].height,
                          ((double)hulls[i][idxH].y - centers[i].y) /
                              (double)temp_rect[i].height));
          }
          idxLeft = idxRight;
          rightSet = false;
        }
      }  // po punktach hull'a
    }    // po konturze
    this->behDescr.resize(centers.size());
    for (size_t i = 0; i < centers.size(); i++) {
      behDescr[i].push_back(PointNorm(centers[i].x, centers[i].y));
      behDescr[i].insert(behDescr[i].end(), points_picture[i].begin(),
                         points_picture[i].end());
    }
  }
}

//--------------------------- Behavior Filtering ------------------------------
void Convex::BehaviorFiltersCheck(Mat frame) {
  for (const auto &obj : detected_objects) {
    Convex_LOG("obj: " + std::to_string(obj->number), LogLevel::kKilo);
    if (obj->behDescr.size() > 0 && !obj->prediction_state && !obj->border) {
      obj->CheckBehavior();
    } else {
      std::string msg = "filters: ";
      for (const auto &filter : obj->bFilter)
        msg += filter->behaviorDescription + " ";
      Convex_LOG(msg, LogLevel::kDetailed);
      if (obj->behDescr.size() <= 0)
        Convex_LOG("obj->behDescr.size() <= 0", LogLevel::kWarning);
      if (obj->prediction_state)
        Convex_LOG("obj->prediction_state==true", LogLevel::kWarning);
      if (obj->border) Convex_LOG("obj->border", LogLevel::kWarning);
    }
    obj->ShowBehaviorStates(frame);
    SaveDetectedBehaviors(obj, frame);
  }
}

void Convex::SaveDetectedBehaviors(detected_object *obj, Mat frame) const {
  vector<bool> behaviorFound = obj->IsFound();
  for (size_t j = 0; j < behaviorFound.size(); j++) {
    if (behaviorFound[j]) {
      if (obj->bFilter[j]->behaviorType >= 2 &&
          obj->bFilter[j]->behaviorType <= 6) {
        putText(frame, obj->message, cvPoint(obj->rect.x, obj->rect.y - 11),
                FONT_HERSHEY_SIMPLEX, 1, Scalar(0, 0, 255), 2);
      } else {
        putText(frame, obj->message, cvPoint(obj->rect.x, obj->rect.y - 11),
                FONT_HERSHEY_SIMPLEX, 1, Scalar(0, 255, 255), 2);
      }
      // Save frame with detected behavior under date name
      if (!obj->eventSaved[j]) {
        Timer stoper{false};
        time_t now = time(0);
        tm *ltm = localtime(&now);
        CreateDirectoryA((LPCSTR) "events", NULL);
        BehaviorDescription beh_descr;
        const std::string behavior = beh_descr.FindBehavior(obj->bFilter[j]->behaviorType).name;
        std::strstream filename;
        filename << "events/" << 1900 + ltm->tm_year << "y"<< 1 + ltm->tm_mon << "m" 
                 << ltm->tm_mday << "d " << ltm->tm_hour << "h"
                 << ltm->tm_min << "m" << ltm->tm_sec << "s_" << behavior
                 << "_obj" << obj->number << ".png" << std::ends;
        Collector::getInstance().detections.emplace_back(filename.str(), frame.clone());        
        Collector::getInstance().AddDataOnce(behavior, obj->number);
        obj->eventSaved[j] = true;
        stoper.PrintElapsed("save_img");
      }
    }
  }
}

}  // namespace bd
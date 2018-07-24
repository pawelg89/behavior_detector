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
#include "..\includes\detected_object.h"
#include "..\includes\logger.h"

static int FrameCounter = 0;
//=============================================================================
//=============================OBSLUGA MYSZY===================================
CvPoint *pt2;  // wskaznik na obiekt przechowujacy aktualnie wybrany punkt
// funkcja obslugi myszy
void MouseEvent2(int ev, int x, int y, int flags, void *param) {
  switch (ev) {
    // odczyt punktu z obrazu po wcisnieciu lewego przycisku myszy
    case CV_EVENT_LBUTTONDOWN:
      pt2->x = x;
      pt2->y = y;
      // cout<<"X = "<<pt->x<<"	Y = "<<pt->y<<endl;
      break;
    default:
      break;
  }
}
void ClearPoint2(CvPoint *p)  // zerowanie wybranego punktu
{
  p->x = -1;
  p->y = -1;
}

namespace bd {
  
using namespace std;
using namespace cv;

int Convex_LOG(std::string msg, LogLevel level, bool new_line = true) {
  return LOG("Convex", msg, level, new_line);
}

Convex::Convex(void)
    : A(5.0), AH_TRESH(10.0), AREA_TRESH(0.01), SALIENCE_TRESH(1.3) {
  char tmp_buff[100] = "";
  morph_size = 2;

  ostrstream zapis(tmp_buff, (int)sizeof(tmp_buff), ios::app);
  if (!load_data("parameters.txt", "method", method)) method = 0;
  zapis.seekp(0);
  zapis << "chosen method: " << method;
  Convex_LOG(tmp_buff, LogLevel::kSetup);

  if (!load_data("parameters.txt", "lPkt", lPkt)) lPkt = 10;
  zapis.seekp(0);
  zapis << "Number of points: " << lPkt;
  Convex_LOG(tmp_buff, LogLevel::kSetup);

  if (!load_data("parameters.txt", "method0", method0)) method0 = 1;
  if (!load_data("parameters.txt", "method1", method1)) method1 = 1;
  if (!load_data("parameters.txt", "method2", method2)) method2 = 1;
  if (!load_data("parameters.txt", "method3", method3)) method3 = 1;
  // Ustawienia dla sekwencji Parking_1
  GHelp = new Helper(0.00523, 0.00294, 0.0045, 5.91, 16.1, 1440, 1080);

  BD = new BehaviorDescription();
  global_counter = 0;
  waitTime = 1;
  logFile.open("logFile.txt");
  this->ClearVectors();
}

Convex::Convex(int frameWidth, int frameHeight, int camID)
    : A(5.0), AH_TRESH(10.0), AREA_TRESH(0.01), SALIENCE_TRESH(1.3) {
  char tmp_buff[100] = "";
  morph_size = 2;

  ostrstream zapis(tmp_buff, (int)sizeof(tmp_buff), ios::app);
  if (!load_data("parameters.txt", "method", method)) method = 0;
  zapis.seekp(0);
  zapis << "chosen method: " << method;
  Convex_LOG(tmp_buff, LogLevel::kSetup);

  if (!load_data("parameters.txt", "lPkt", lPkt)) lPkt = 10;
  zapis.seekp(0);
  zapis << "Number of points: " << lPkt;
  Convex_LOG(tmp_buff, LogLevel::kSetup);

  if (!load_data("parameters.txt", "method0", method0)) method0 = 1;
  if (!load_data("parameters.txt", "method1", method1)) method1 = 1;
  if (!load_data("parameters.txt", "method2", method2)) method2 = 1;
  if (!load_data("parameters.txt", "method3", method3)) method3 = 1;

  GHelp = new Helper(0.00523, 0.00294, 0.0045, 5.91, 16.1, 1440, 1080);

  global_counter = 0;
  waitTime = 1;
  logFile.open("logFile.txt");
  this->ClearVectors();
}

Convex::~Convex(void) {}

static int GloCTR = 0;

void Convex::SHIELD(Mat frame, Mat fore, int view) {
  Convex_LOG("SHIELD(Mat frame, Mat fore, int view) called.", LogLevel::kMega);
  GloCTR++;
  this->NCM(frame, fore);

  this->track_objects(new IplImage(frame), view);

  this->BehaviorFiltersCheck(frame);
  imshow("frame", frame);
  imshow("fore", fore);
  this->ClearVectors();
}

void Convex::SHIELD(Mat frame, Mat fore, bool creatingDescriptors) {
  GloCTR++;
  // Wyznaczenie hulli, oraz punktow do deskryptora dla kazdego obiektu
  this->NCM(frame, fore);

  setMouseCallback("Create Descriptor Window", MouseEvent2);

  // Obsluga dodawania punktow do deskryptora
  pt2 = new CvPoint();
  ClearPoint2(pt2);

  imshow("Create Descriptor Window", frame);
  char _char = cvWaitKey(this->waitTime);

  if (_char == 'p' || _char == 'P') {
    if (waitTime == 1)
      waitTime = 0;
    else
      waitTime = 1;
  }
  // if(GloCTR == 2631 || GloCTR == 2633 || GloCTR == 2636 || GloCTR == 2641 ||
  // GloCTR == 2643 || GloCTR == 2646 || GloCTR == 2663 || GloCTR == 2664 ||
  // GloCTR == 2666 || GloCTR == 2671 )//&& GloCTR%2 == 0)  if(GloCTR >= 616 &&
  // GloCTR <= 688 && GloCTR%2 == 0)
  ////if(GloCTR >= 616 && GloCTR <= 664 && GloCTR%2 == 0)
  //{
  //	pt2->x = 300;
  //	pt2->y = 300;
  //}
  if (pt2->x != -1) {
    std::vector<PointNorm> asd;
    // finding nearest
    double max_dist = 10000000000;
    int nearest_index;
    for (size_t i = 0; i < this->centers.size(); i++) {
      if (this->GetDist(*pt2, this->centers[i]) < max_dist) {
        max_dist = this->GetDist(*pt2, this->centers[i]);
        nearest_index = i;
      }
    }
    int frameW = frame.cols;
    int frameH = frame.rows;

    frame.adjustROI(
        -temp_rect[nearest_index].y,
        (temp_rect[nearest_index].y + temp_rect[nearest_index].height) - frameH,
        -temp_rect[nearest_index].x,
        (temp_rect[nearest_index].x + temp_rect[nearest_index].width) - frameW);
    char fileName_buff[100];
    sprintf(fileName_buff, "descr/%d.bmp", GloCTR);
    imshow("cut", frame);
    cvWaitKey(10);
    imwrite(fileName_buff, frame);

    for (size_t i = 0; i < this->behDescr[nearest_index].size(); i++) {
      asd.push_back(this->behDescr[nearest_index][i]);
    }
    this->BD->descriptor.push_back(asd);
    this->BD->v_sizes.push_back((int)asd.size());
  }
  // if(GloCTR == 689)
  //{
  //	_char = 'e';
  //}
  if (_char == 'e' || _char == 'E') {
    BD->sizes = new int[BD->descriptor.size()];
    for (size_t i = 0; i < BD->descriptor.size(); i++)
      BD->sizes[i] = BD->v_sizes[i];

    BD->SaveBehaviorDescriptor("desc");
    BD->descriptor.clear();
    delete[] BD->sizes;
  }

  // Trzeba pamietac o czyszczeniu wektorow
  this->ClearVectors();
}

void Convex::ClearVectors() {
  contours.clear();
  hulls.clear();
  NCMs.clear();
  contour_areas.clear();

  temp_pos.clear();
  temp_rect.clear();
  behDescr.clear();

  if (GloCTR == 8300 || GloCTR == 4500) detected_objects.clear();
}

void Convex::NCM(Mat frame, Mat frame_gray, bool visualize) {
  /*CvFont font;
  cvInitFont(&font, CV_FONT_VECTOR0, 0.5, 0.5, 0, 1);*/
  // if(cvWaitKey(1) == 's')
  //	global_saveContours = !global_saveContours;

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

  /// Find contours
  // Scalar color = Scalar(255, 255, 0);
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
  // logFile<<"Frame: "<<GloCTR<<", objects in scene: "<<contours.size()<<",
  // contours sizes:";  for(int i=0; i<contours.size();i++)
  //{
  //	logFile<<" "<<contours[i].size();
  //}
  // logFile<<endl;
  // Przygotowanie wektorow dla Kalman Filter
  temp_rect.resize(contours.size());
  temp_pos.resize(contours.size());
  centers.resize(contours.size());

  /*char pos_buff[100] = "";
  ostrstream zapis(pos_buff, (int)sizeof(pos_buff), ios::app);
*/
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

  // Pozycja, gorny lewy rog konturu
  // for (size_t i = 0; i < contours.size(); i++) {
  //  //	temp_pos[i]=contours[i][0];
  //  temp_rect[i].x = temp_pos[i].x;
  //  temp_rect[i].y = temp_pos[i].y;
  //}
  // for (size_t i = 0; i < temp_pos.size(); i++) {
  //  temp_pos[i].x = temp_rect[i].x + temp_rect[i].width / 2;
  //  temp_pos[i].y = temp_rect[i].y + temp_rect[i].height;
  //}
  // Progowanie sylwetki
  // for (size_t i = 0; i < contours.size(); i++)
  //  for (size_t j = 0; j < contours[i].size(); j++)
  //    circle(frame_gray, contours[i][j], 0, 255, 1);
  threshold(frame_gray, frame_gray, 1, 255, CV_THRESH_BINARY);

  //------------------------------------------------------------------------------------------------
  // Wyliczamy powierzchnie konturów
  contour_areas.resize(contours.size());
  for (size_t i = 0; i < contours.size(); i++)
    contour_areas[i] = contourArea(contours[i]);

  // color = Scalar(255, 255, 0);
  // vector<vector<Point>> hull(contours.size());
  hulls.resize(contours.size());
  for (size_t i = 0; i < contours.size(); i++)
    convexHull(Mat(contours[i]), hulls[i], true);

  NCMs.resize(contours.size());
  int number_iterations = 2;
  for (int iter = 0; iter < number_iterations; iter++) {
    /*color = Scalar((iter + 1) * 255 / number_iterations,
                   (iter + 1) * 255 / number_iterations, 0);*/
    for (size_t i = 0; i < contours.size(); i++) {
      vector<int> ncm;
      int NCM_counter = 1;
      int previdx = hulls[i].size() - 1;
      Point prev = hulls[i][previdx];
      Point curr = hulls[i][previdx];
      // bool found2 = false;
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
            // char buffer[10];
            // putText(frame,itoa(NCM_counter,buffer,10),nbr_pt,
            // FONT_HERSHEY_SIMPLEX,0.3,cvScalar(255,255,0));
            // circle(frame,farthest_point,2,color,1);
            NCM_counter++;

            ncm.push_back(idx);

            if (number_iterations > 1) {
              // Debug
              // circle(frame,prev,2,cvScalar(255,0,0),1);//blue poczatek
              // circle(frame,curr,2,cvScalar(0,255,0),1);//green koniec
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
  Scalar yellow = Scalar(255, 255, 0);
  Scalar green = Scalar(0, 255, 0);
  char pos_buff[100] = "";
  ostrstream zapis(pos_buff, (int)sizeof(pos_buff), ios::app);
  for (size_t i = 0; i < centers.size(); ++i) {
    zapis.seekp(0);
    zapis << "[" << centers[i].x << "," << centers[i].y << "]";
    putText(frame, pos_buff, temp_pos[i], font.font_face, 0.35, green);
  }
  for (size_t i = 0; i < contours.size(); i++) {
    drawContours(frame, hulls, i, yellow, 1, 8, vector<Vec4i>(), 0, Point());
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

int Convex::track_objects(IplImage *img, int c) {
  CvFont font;
  cvInitFont(&font, CV_FONT_VECTOR0, 1, 1, 0, 2);
  int counter = -1;
  int counter2 = -1;
  double max_dist = (double)INT_MAX;

  //	char buffer[10];
  double thrsh = 0.1;
  vector<int> points_mask;
  vector<int> objects_mask;
  double x = 0;
  double y = 0;
  double dist = 0;

  double normal_dist = 100;   // maksymalna odleglosc dla obiektow powiazywanych
  double marge_dist = 50;     // odleglosc ponizej ktorej nastepuje scalenie
  double dismarge_dist = 50;  // odleglosc powyzej ktorej nastepuje rozklejenie
  //
  int trash_low_tresh = 1;
  int trash_high_tresh = 40;
  int max_prediction_length = 40;
  int min_presentation_time = 0;

  for (int i = 0; i < (int)temp_pos.size(); i++) {
    points_mask.push_back(0);
  }
  for (int i = 0; i < (int)detected_objects.size(); i++) {
    objects_mask.push_back(0);
  }

  for (int i = 0; i < (int)temp_pos.size(); i++) {
    for (int j = 0; j < (int)detected_objects.size(); j++) {
      if (objects_mask[j] == 0)  // czy dany obiekt nie zostal juz przypisany
      {
        if ((int)detected_objects[j]->marged_objects.size() == 0) {
          x = pow(double(detected_objects[j]->next_pos.x - temp_pos[i].x), 2);
          y = pow(double(detected_objects[j]->next_pos.y - temp_pos[i].y), 2);
          dist = sqrt(x + y);
        } else {
          x = pow(
              double(detected_objects[j]->KFilter->statePt.x - temp_pos[i].x),
              2);
          y = pow(
              double(detected_objects[j]->KFilter->statePt.y - temp_pos[i].y),
              2);
          dist = sqrt(x + y);
        }
        if (dist < max_dist && dist < normal_dist) {
          max_dist = dist;
          counter = i;
          counter2 = j;
        }
      }
    }
    if (counter2 > -1) {
      for (int k = 0; k < (int)temp_pos.size(); k++) {
        if (points_mask[k] == 0) {
          x = pow(
              double(detected_objects[counter2]->next_pos.x - temp_pos[k].x),
              2);
          y = pow(
              double(detected_objects[counter2]->next_pos.y - temp_pos[k].y),
              2);
          dist = sqrt(x + y);
          if (dist < max_dist) {
            max_dist = dist;
            counter = k;
          }
        }
      }
    }
    if (counter2 > -1) {
      detected_objects[counter2]->KFilter->Action(temp_pos[counter]);
      detected_objects[counter2]->next_pos = temp_pos[counter];
      detected_objects[counter2]->rect = temp_rect[counter];
      detected_objects[counter2]->prediction_life_time = 0;
      detected_objects[counter2]->prediction_state = false;
      detected_objects[counter2]->behDescr = behDescr[counter];
      detected_objects[counter2]->is_moving =
          detected_objects[counter2]->detect_movement();
      if (!detected_objects[counter2]->is_moving)
        circle((Mat)img,
               cvPoint(detected_objects[counter2]->rect.x +
                           detected_objects[counter2]->rect.width,
                       detected_objects[counter2]->rect.y),
               10, cvScalar(255, 255, 64));
      if ((int)detected_objects[counter2]->KFilter->kalmanv.size() >
          min_presentation_time) {
        if (detected_objects[counter2]->number > -1)
          cvPutText(img,
                    std::to_string(detected_objects[counter2]->number).c_str(),
                    detected_objects[counter2]->KFilter->measPt, &font,
                    cvScalar(255, 0, 0));
        if (detected_objects[counter2]->is_inside_restricted_area == false)
          rectangle((Mat)img, detected_objects[counter2]->rect,
                    cvScalar(0, 255, 0), 1, 8, 0);
        else
          rectangle((Mat)img, detected_objects[counter2]->rect,
                    cvScalar(0, 255, 255), 2, 8, 0);
        detected_objects[counter2]->KFilter->print(img);  //,false);
      }

      /////////////////////////////////////////////////////////////////////////
      if (detected_objects[counter2]->next_pos.x < 1 ||
          detected_objects[counter2]->next_pos.x > img->width - 2 ||
          detected_objects[counter2]->next_pos.y < 1 ||
          detected_objects[counter2]->next_pos.y > img->height - 2) {
        detected_objects[counter2]->border = true;
      } else
        detected_objects[counter2]->border = false;
      ///////////////////////////////////////////////////////////////////////////
      points_mask[counter] = 1;
      objects_mask[counter2] = 1;
      counter2 = -1;
      counter = -1;
      max_dist = INT_MAX;
    }
  }

  // zostaly obiekty
  for (int i = 0; i < (int)detected_objects.size(); i++) {
    if (objects_mask[i] == 0)  // niepowizane
    {
      // usowanie przy krawedziach
      if (detected_objects[i]->next_pos.x < thrsh * img->width ||
          detected_objects[i]->next_pos.x > (1 - thrsh) * img->width ||
          detected_objects[i]->next_pos.y < thrsh * img->height ||
          detected_objects[i]->next_pos.y > (1 - thrsh) * img->height) {
        delete detected_objects[i];
        detected_objects.erase(detected_objects.begin() + i);
        objects_mask.erase(objects_mask.begin() + i);
        i--;
        continue;
      } else  // zostaly na srodku - sklejenie/zgubienie
      {
        if (detected_objects[i]->prediction_life_time >
            max_prediction_length)  // zbyt dluga predykcja
        {
          delete detected_objects[i];
          detected_objects.erase(detected_objects.begin() + i);
          objects_mask.erase(objects_mask.begin() + i);
          i--;
          continue;
        }
        if ((int)detected_objects[i]->KFilter->kalmanv.size() <
                trash_high_tresh &&
            (int)detected_objects[i]->KFilter->kalmanv.size() >
                trash_low_tresh)  // smieci
        {
          delete detected_objects[i];
          detected_objects.erase(detected_objects.begin() + i);
          objects_mask.erase(objects_mask.begin() + i);
          i--;
          continue;
        }
        bool marged = false;
        int marged_with = -1;
        max_dist = INT_MAX;
        // marge detection
        for (int k = 0; k < (int)detected_objects.size(); k++) {
          if (objects_mask[k] == 1)  // tylko dla sledzonych obiektow obiektow
          {
            x = pow(double(detected_objects[i]->next_pos.x -
                           detected_objects[k]->next_pos.x),
                    2);
            y = pow(double(detected_objects[i]->next_pos.y -
                           detected_objects[k]->next_pos.y),
                    2);
            dist = sqrt(x + y);
            if (dist < max_dist && dist < marge_dist) {
              max_dist = dist;
              marged_with = k;
            }
          }
        }
        if (marged_with != -1) {
          marged = true;
        }
        if (marged)  // sklejenie
        {
          detected_objects[marged_with]->marged_objects.push_back(
              detected_objects[i]);
          detected_objects.erase(detected_objects.begin() + i);
          objects_mask.erase(objects_mask.begin() + i);
          i--;
        } else  // zgubienie
        {
          if (detected_objects[i]->is_moving) {
            if (!detected_objects[i]->direction_estimated)
              detected_objects[i]->estimate_direction();
            detected_objects[i]->KFilter->Action(
                cvPoint((int)(detected_objects[i]->next_pos.x +
                              detected_objects[i]->x_movement),
                        (int)(detected_objects[i]->next_pos.y +
                              detected_objects[i]->y_movement)));
          }
          detected_objects[i]->prediction_state =
              true;  // przesuniecie bialego prostokata
          detected_objects[i]->prediction_life_time++;
          detected_objects[i]->next_pos = detected_objects[i]->KFilter->measPt;
          detected_objects[i]->rect.x = detected_objects[i]->next_pos.x -
                                        detected_objects[i]->rect.width / 2;
          detected_objects[i]->rect.y = detected_objects[i]->next_pos.y -
                                        detected_objects[i]->rect.height / 2;
          if ((int)detected_objects[i]->KFilter->kalmanv.size() >
              min_presentation_time) {
            //		cvPutText(img,itoa(detected_objects[i]->number,buffer,10),detected_objects[i]->next_pos,&font,cvScalar(0,0,255));
            rectangle((Mat)img, detected_objects[i]->rect,
                      cvScalar(255, 255, 255), 1, 8, 0);
          }
        }
      }
    }
  }
  // przesuniecie scalonych obiektow
  for (int i = 0; i < (int)detected_objects.size(); i++) {
    for (int j = 0; j < (int)detected_objects[i]->marged_objects.size(); j++) {
      if ((int)detected_objects[i]->marged_objects[j]->KFilter->kalmanv.size() <
              trash_high_tresh &&
          (int)detected_objects[i]->marged_objects[j]->KFilter->kalmanv.size() >
              trash_low_tresh)  // smieci
      {
        delete detected_objects[i]->marged_objects[j];
        detected_objects[i]->marged_objects.erase(
            detected_objects[i]->marged_objects.begin() + j);
        continue;
      }
      if (detected_objects[i]->marged_objects[j]->prediction_life_time >
          max_prediction_length)  // zbyt dluga predykcja
      {
        delete detected_objects[i]->marged_objects[j];
        detected_objects[i]->marged_objects.erase(
            detected_objects[i]->marged_objects.begin() + j);
        continue;
      }

      if (detected_objects[i]->marged_objects[j]->is_moving) {
        if (!detected_objects[i]->marged_objects[j]->direction_estimated)
          detected_objects[i]->marged_objects[j]->estimate_direction();
        detected_objects[i]->marged_objects[j]->KFilter->Action(
            cvPoint((int)(detected_objects[i]->marged_objects[j]->next_pos.x +
                          detected_objects[i]->marged_objects[j]->x_movement),
                    (int)(detected_objects[i]->marged_objects[j]->next_pos.y +
                          detected_objects[i]->marged_objects[j]->y_movement)));
      }
      detected_objects[i]->marged_objects[j]->prediction_state = true;
      detected_objects[i]->marged_objects[j]->prediction_life_time++;
      detected_objects[i]->marged_objects[j]->next_pos =
          detected_objects[i]->marged_objects[j]->KFilter->measPt;
      detected_objects[i]->marged_objects[j]->rect.x =
          detected_objects[i]->marged_objects[j]->next_pos.x -
          detected_objects[i]->marged_objects[j]->rect.width / 2;
      detected_objects[i]->marged_objects[j]->rect.y =
          detected_objects[i]->marged_objects[j]->next_pos.y -
          detected_objects[i]->marged_objects[j]->rect.height / 2;

      if ((int)detected_objects[i]->marged_objects[j]->KFilter->kalmanv.size() >
          min_presentation_time) {
        //	cvPutText(img,itoa(detected_objects[i]->marged_objects[j]->number,buffer,10),detected_objects[i]->marged_objects[j]->next_pos,&font,cvScalar(0,0,255));
        rectangle((Mat)img, detected_objects[i]->marged_objects[j]->rect,
                  cvScalar(255, 255, 0), 1, 8, 0);
      }
    }
  }

  for (int i = 0; i < (int)temp_pos.size(); i++) {
    if (points_mask[i] == 0)  // niepowizane punkty
    {
      bool dismarged = false;
      int dismarged_from = -1;
      int dismarged_index = -1;
      max_dist = INT_MAX;
      for (int j = 0; j < (int)detected_objects.size(); j++) {
        for (int k = 0; k < (int)detected_objects[j]->marged_objects.size();
             k++) {
          x = pow(double(temp_pos[i].x -
                         detected_objects[j]->marged_objects[k]->next_pos.x),
                  2);
          y = pow(double(temp_pos[i].y -
                         detected_objects[j]->marged_objects[k]->next_pos.y),
                  2);
          dist = sqrt(x + y);
          if (dist < max_dist && dist < dismarge_dist) {
            max_dist = dist;
            dismarged_from = j;
            dismarged_index = k;
          }
        }
      }
      if (dismarged_from != -1) {
        dismarged = true;
      }
      if (dismarged) {
        detected_objects.push_back(
            detected_objects[dismarged_from]->marged_objects[dismarged_index]);
        detected_objects[dismarged_from]->marged_objects.erase(
            detected_objects[dismarged_from]->marged_objects.begin() +
            dismarged_index);
        detected_objects[detected_objects.size() - 1]->KFilter->Action(
            temp_pos[i]);
        detected_objects[detected_objects.size() - 1]->next_pos = temp_pos[i];
        detected_objects[detected_objects.size() - 1]->rect = temp_rect[i];
        detected_objects[detected_objects.size() - 1]->behDescr = behDescr[i];
        detected_objects[detected_objects.size() - 1]->prediction_life_time = 0;
        detected_objects[detected_objects.size() - 1]->prediction_state = false;
        detected_objects[detected_objects.size() - 1]->direction_estimated =
            false;
        if ((int)detected_objects[detected_objects.size() - 1]
                ->KFilter->kalmanv.size() > min_presentation_time) {
          //		cvPutText(img,itoa(detected_objects[detected_objects.size()-1]->number,buffer,10),detected_objects[(int)detected_objects.size()-1]->KFilter->measPt,&font,cvScalar(0,0,255));
          rectangle((Mat)img,
                    detected_objects[detected_objects.size() - 1]->rect,
                    cvScalar(0, 255, 0), 1, 8, 0);
        }
        points_mask[i] = 1;

      } else {
        detected_object *temp_obj = new detected_object();
        temp_obj->camera = c;
        temp_obj->current_pos = temp_pos[i];
        temp_obj->rect = temp_rect[i];
        temp_obj->KFilter->Initialize(temp_pos[i]);
        temp_obj->KFilter->Action(temp_pos[i]);
        temp_obj->KFilter->Action(temp_pos[i]);
        temp_obj->next_pos = temp_obj->KFilter->statePt;
        global_counter++;
        temp_obj->number = -1;  // global_counter;
        temp_obj->SetObjNumber();
        detected_objects.push_back(temp_obj);
      }
    }
  }

  for (size_t i = 0; i < detected_objects.size(); i++) {
    detected_objects[i]->is_close_to = false;

    if (detected_objects[i]->marged_objects.size() > 0) {
      for (size_t j = 0; j < detected_objects[i]->marged_objects.size(); j++)
        detected_objects[i]->marged_objects[j]->is_close_to = true;
      detected_objects[i]->is_close_to = true;
      continue;
    }
    for (size_t j = 0; j < detected_objects.size(); j++) {
      if (i == j) continue;
      detected_objects[i]->is_close_to = is_close_to(i, j);
      if (detected_objects[i]->is_close_to) break;
    }
  }
  for (size_t i = 0; i < detected_objects.size(); i++) {
    if (detected_objects[i]->is_close_to)
      cvCircle(
          img,
          cvPoint(detected_objects[i]->rect.x, detected_objects[i]->rect.y), 2,
          cvScalar(255, 255, 0));
    for (size_t j = 0; j < detected_objects[i]->marged_objects.size(); j++) {
      if (detected_objects[i]->marged_objects[j]->is_close_to)
        cvCircle(img,
                 cvPoint(detected_objects[i]->marged_objects[j]->rect.x,
                         detected_objects[i]->rect.y),
                 2, cvScalar(255, 255, 0));
    }
  }
  // temp_pos.clear();
  // temp_rect.clear();
  return 0;
}

bool Convex::is_close_to(int master_index, int slave_index) {
  int thrsh;
  double multiplier = 1.5;
  double x;
  double y;
  x = (detected_objects[master_index]->next_pos.x -
       detected_objects[slave_index]->next_pos.x) *
      (detected_objects[master_index]->next_pos.x -
       detected_objects[slave_index]->next_pos.x);
  y = (detected_objects[master_index]->next_pos.y -
       detected_objects[slave_index]->next_pos.y) *
      (detected_objects[master_index]->next_pos.y -
       detected_objects[slave_index]->next_pos.y);
  thrsh =
      static_cast<int>(multiplier * detected_objects[master_index]->rect.width);
  if (sqrt(x + y) < thrsh) {
    return true;
  }
  return false;
}

bool Convex::is_background_ok(int width, int height) {
  double area = width * height;
  double area_thrsh = 0.6;
  double accu = 0;
  for (size_t i = 0; i < detected_objects.size(); i++) {
    accu += detected_objects[i]->rect.area();
  }
  if (accu / area < area_thrsh) return true;
  detected_objects.clear();
  return false;
}

double Rest(double x, double y) {
  int tempX = (int)x;
  int tempY = (int)y;

  return (double)(x / y - tempX / tempY);
}

void Convex::BehaviorInput(Mat frame, vector<vector<Point>> hulls) {
  if (method0) {
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
  if (method1) {
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

  if (method2) {
    // N punktów konturu wybieranych równomiernie

    vector<vector<PointNorm>> points_picture;
    points_picture.resize((int)contours.size());

    for (int i = 0; i < (int)contours.size(); i++) {
      int lPktKonturu = (int)contours[i].size();
      // Na wypadek zbyt du¿ej iloœci punktów
      if (lPkt > lPktKonturu) break;

      int step = lPktKonturu / lPkt;
      double buffer = 0;  // Rest((double)lPktKonturu,(double)lPkt);
      int stepCTR = 0;
      for (int j = 0; j < lPktKonturu && stepCTR < lPkt;
           j += (step + (int)buffer)) {
        PointNorm tempP;
        tempP.x = (double)((contours[i][j].x - centers[i].x) /
                           (double)temp_rect[i].height);
        tempP.y = (double)((contours[i][j].y - centers[i].y) /
                           (double)temp_rect[i].height);
        stepCTR++;
        points_picture[i].push_back(tempP);

        if (buffer < 1.0) {
          buffer += Rest((double)lPktKonturu, (double)lPkt);
        } else {
          buffer -= 1.0;
          buffer += Rest((double)lPktKonturu, (double)lPkt);
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
  if (method3) {  // Wersja do sprawdzenia, czy dobrze jest obslugiwana
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
  for (size_t i = 0; i < detected_objects.size(); i++) {
    if (detected_objects[i]->behDescr.size() > 0 &&
        !detected_objects[i]->prediction_state && !detected_objects[i]->border)
      detected_objects[i]->CheckBehavior();
  }
  for (size_t i = 0; i < detected_objects.size(); i++) {
    for (size_t k = 0; k < detected_objects.size(); k++) {
      detected_objects[i]->ShowBehaviorStates(frame);
    }

    vector<bool> behaviorFound = detected_objects[i]->IsFound();
    for (size_t j = 0; j < behaviorFound.size(); j++)
      if (behaviorFound[j]) {
        std::string msg = "Detected: " + detected_objects[i]->message;
        Convex_LOG(msg, LogLevel::kKilo);
        if (detected_objects[i]->bFilter[j]->behaviorType >= 2 &&
            detected_objects[i]->bFilter[j]->behaviorType <= 6)
          putText(frame, detected_objects[i]->message,
                  cvPoint(detected_objects[i]->rect.x,
                          detected_objects[i]->rect.y - 11),
                  FONT_HERSHEY_SIMPLEX, 1, Scalar(0, 0, 255), 2);
        else
          putText(frame, detected_objects[i]->message,
                  cvPoint(detected_objects[i]->rect.x,
                          detected_objects[i]->rect.y - 11),
                  FONT_HERSHEY_SIMPLEX, 1, Scalar(0, 255, 255), 2);
        // Save frame with detected behavior under date name
        if (!detected_objects[i]->eventSaved[j]) {
          char namebuffer[100];
          time_t now = time(0);
          tm *ltm = localtime(&now);

          CreateDirectoryA((LPCSTR) "events", NULL);
          sprintf(namebuffer, "events/%dy%dm%dd %dh%dm%ds.png",
                  1900 + ltm->tm_year, 1 + ltm->tm_mon, ltm->tm_mday,
                  ltm->tm_hour, ltm->tm_min, ltm->tm_sec);
          cvSaveImage(namebuffer, new IplImage(frame));
          // DB->EventSave(j+1, temp);
          // cvReleaseImage(&temp);
          detected_objects[i]->eventSaved[j] = true;
        }
      }
  }
}
}  // namespace bd
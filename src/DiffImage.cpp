#include "..\includes\DiffImage.h"
// STL includes
#include <exception>
#include <fstream>
#include <iostream>
#include <string>
#include <strstream>
// OpenCV includes
#include <opencv\cv.h>
#include <opencv2\gpu\gpu.hpp>
#include <opencv2\highgui.hpp>
#include <opencv2\video\background_segm.hpp>
// Project includes
#include "..\includes\collector.h"
#include "..\includes\Convex.h"
#include "..\includes\GCapture.h"
#include "..\includes\logger.h"
#include "..\includes\marked_frame.h"
#include "..\includes\timer.h"

int global_counter = 0;
static bool START = false;
std::vector<cv::Point2f> vec;
void MouseEvent(int ev, int x, int y, int flags, void *param) {
  switch (ev) {
    // odczyt punktu z obrazu po wcisnieciu lewego przycisku myszy
    case CV_EVENT_LBUTTONDOWN:
      vec.push_back(cvPoint(x, y));
      break;
    case CV_EVENT_RBUTTONDOWN:
      for (int i = 0; i < (int)vec.size(); i++) {
        if ((vec[i].x > (x - 15) && vec[i].x < (x + 15)) &&
            (vec[i].y > (y - 15) && vec[i].y < (y + 15))) {
          vec.erase(vec.begin() + i);
          break;
        }
      }
      break;
    default:
      break;
  }
}

void ClearPoint(CvPoint *p) {
  p->x = -1;
  p->y = -1;
}

namespace bd {

using namespace cv;
using namespace std;

int DI_LOG(const std::string &msg, const LogLevel level) {
  return LOG("DiffImage", msg, level);
}

DiffImage::DiffImage(char *file) { file_name = file; }

inline double GetDist(Point2f p1, Point2f p2) {
  return sqrt(((p1.x - p2.x) * (p1.x - p2.x)) +
              ((p1.y - p2.y) * (p1.y - p2.y)));
}

DiffImage::~DiffImage(void) {}

void DiffImage::DiffImageAction() {
  DI_LOG(file_name, LogLevel::kSetup);
  int frameCounter = 0;
  CvFont font;
  cvInitFont(&font, CV_FONT_VECTOR0, 0.5, 0.5, 0, 1);

  int mode;
  if (!load_data("parameters.txt", "mode", mode)) mode = 5;

  std::cout << "Chosen moode: " << mode << std::endl;
  std::string temp_msg = "Chosen mode: ";
  temp_msg += std::to_string(mode);
  DI_LOG(temp_msg, LogLevel::kSetup);

  long double mean_time = 0;
  int tick_count = 0;
  long double ms;
  unsigned __int64 freq, start, stop;
  QueryPerformanceFrequency(reinterpret_cast<LARGE_INTEGER *>(&freq));

  int scale = 1;
  int current_threshold = 135;
  int nbr_dot = 0;
  long double dot_time = 0;

  if (mode != 7) {
    cvNamedWindow("Original Video", 0);
    // cvMoveWindow("Original Video",-1280,176);
    // cvResizeWindow("Original Video",1260,980);
  } else {
    namedWindow("Create Descriptor Window", CV_WINDOW_NORMAL);
    // moveWindow("Create Descriptor Window",0,-1024);
  }

  CvCapture *video = cvCaptureFromAVI(file_name);

  // IplImage* frame;
  Mat mat_frame;
  // cap.read(mat_frame);
  // frame = new IplImage(mat_frame);

  IplImage *frame = cvQueryFrame(video);
  if (frame == NULL || frame->nSize < 0) {
    throw(string) "DiffImage: Couldn't open video file. Check path: " +
        file_name;
  }
  // Inicjalizacja odejmowania tla, zmienne potrzebne do MOG2
  float mogThreshold;
  if (!load_data("parameters.txt", "mogThreshold", mogThreshold))
    mogThreshold = 50.0;

  BackgroundSubtractorMOG2 *bg =
      new BackgroundSubtractorMOG2(10000, mogThreshold, true);
  //	bg->fTau = 0.1;
  // if(!load_data("parameters.txt","backgroundRatio",bg->backgroundRatio))
  //	DI_LOG("Couldnt find parameter <backgroundRatio>.",1);
  Mat fore;

  Convex *convex = new Convex();
  int x_size, y_size;
  char _char;
  int waitTime = 1;

  DI_LOG("Entering main loop.", LogLevel::kDefault);
  do {
    QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER *>(&start));

    x_size = frame->width;
    y_size = frame->height;

    bg->operator()((Mat)frame, fore);

    threshold(fore, fore, current_threshold, 255, CV_THRESH_BINARY);

    /*char nameBuffer[100];
    sprintf(nameBuffer,"rameczki/%d.bmp",frameCounter);
    cvSaveImage(nameBuffer,frame);*/

    if (convex->is_background_ok(frame->width, frame->height) && _char != 'r' &&
        _char != 'R' /*&&
			frameCounter!= 1421 && frameCounter!= 2270 && frameCounter!= 2990 && frameCounter!= 3810 && frameCounter!= 4490 && frameCounter!= 5150 && frameCounter!= 5960 && 
			frameCounter!= 6790 && frameCounter!= 7430 && frameCounter!= 8150 && frameCounter!= 8950 && frameCounter!= 9700 && frameCounter!= 10560 && frameCounter!= 11400 &&
			frameCounter!= 12020*/) {
      if (mode == 5) convex->SHIELD((Mat)frame, fore, 0);

      cvPutText(frame, std::to_string(frameCounter).c_str(), cvPoint(10, 10),
                &font, cvScalar(0, 0, 255));
      frameCounter++;

      if (mode == 7) convex->SHIELD((Mat)frame, fore, true);
    }
    // else
    //{
    // cvPutText(frame,itoa(frameCounter,bufferChar,10) ,
    // cvPoint(10,10),&font,cvScalar(0,0,255)); frameCounter++; delete
    // bg;//>~BackgroundSubtractorMOG2(); bg = new
    // BackgroundSubtractorMOG2(10000, mogThreshold,true); cout<<"Background
    // reset"<<endl;
    //}

    // cvPutText(frame,itoa(frameCounter,bufferChar,10) ,
    // cvPoint(10,10),&font,cvScalar(0,0,255));  frameCounter++;

    if (frame->roi) cvResetImageROI(frame);

    QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER *>(&stop));
    ms = (static_cast<long double>(stop) - start) / freq * 1000;
    if (mode != 7) cvShowImage("Original Video", frame);
    std::cout << ms << endl;

    char nameBuffer[100];
    sprintf(nameBuffer, "rameczki/%d.bmp", frameCounter);
    // cvSaveImage(nameBuffer,frame);

    if (mode != 7) _char = cvWaitKey(waitTime);
    if (_char == 'r' || _char == 'R') convex->detected_objects.clear();
    if (_char == 'n' || _char == 'N') cvSaveImage(nameBuffer, frame);
    if (_char == 'p')
      if (waitTime == 1)
        waitTime = 0;
      else
        waitTime = 1;

    frame = cvQueryFrame(video);
  } while (_char != 'q' && frame);

  DI_LOG("Leaving main loop. Calling destructors...", LogLevel::kDefault);
  cvReleaseCapture(&video);
  cvDestroyAllWindows();
  std::cout << std::endl;
}

void load_marker_coord(vector<vector<Point2f>> &marker_coord,
                       int camera_number) {
  ifstream file2;
  file2.open("D:\\marker_coord.txt");
  file2 >> camera_number;

  for (int i = 0; i < camera_number; i++) {
    vector<Point2f> temp_vec;
    for (int j = 0; j < 4; j++) {
      Point2f temp;
      file2 >> temp.x;
      file2 >> temp.y;

      temp_vec.push_back(temp);
    }

    marker_coord.push_back(temp_vec);
  }
  file2.close();
}

void MakeWindow(const std::string &name, const cv::Point2i size,
                const cv::Point2i position) {
  namedWindow(name, CV_WINDOW_NORMAL);
  resizeWindow(name, size.x, size.y);
  moveWindow(name, position.x, position.y);
}

void DiffImage::DiffImageAction2() {
  // zmienne £ukasza
  vector<vector<Point2f>> marker_coord;
  vector<Mat> H_f;
  load_marker_coord(marker_coord, 4);
  ///////////////////////////////////////////////////
  int frameCounter = 0;
  int mode;
  if (!load_data("parameters.txt", "mode", mode)) mode = 5;
  std::cout << "Chosen moode: " << mode << std::endl;
  char temp_msg[100] = {"Chosen mode: "};
  std::string temp_mode = std::to_string(mode);
  // char temp_mode[10];
  // itoa(mode,temp_mode, 10);
  strcat(temp_msg, temp_mode.c_str());
  DI_LOG(temp_msg, LogLevel::kSetup);

  long double mean_time = 0;
  int tick_count = 0;
  long double ms;
  unsigned __int64 freq, start, stop;
  QueryPerformanceFrequency(reinterpret_cast<LARGE_INTEGER *>(&freq));

  int scale = 1;
  int current_threshold = 135;
  int nbr_dot = 0;
  long double dot_time = 0;

  //---------------------------------------------------------------------------
  //------------------------- WCZYTYWANIE KAMER -------------------------------
  int nbrViews = 0;
  load_data("parameters.txt", "nbrViews", nbrViews);
  char pathDirX[1000] = "";
  // hehe legacy stream name ];->
  ostrstream monkeys(pathDirX, (int)sizeof(pathDirX), ios::app);

  vector<string> pathDir;
  string tempDir;
  for (int i = 0; i < nbrViews; i++) {
    monkeys.seekp(0);
    monkeys << "pathDir" << i << ends;
    load_data("parameters.txt", pathDirX, tempDir);
    pathDir.push_back(tempDir);
    if (mode != 7) {
      auto win_size = cv::Point2i(400, 300);
      int win_x = 405;
      int win_y = 333;
      if (i == 0) MakeWindow(tempDir, win_size, cv::Point2i(1, 1));
      if (i == 1) MakeWindow(tempDir, win_size, cv::Point2i(win_x, 1));
      if (i == 2) MakeWindow(tempDir, win_size, cv::Point2i(1, win_y));
      if (i == 3) MakeWindow(tempDir, win_size, cv::Point2i(win_x, win_y));
      MakeWindow("fore", win_size, cv::Point2i(1, win_y));
    }
    if (i == 0) {
      cvSetMouseCallback(tempDir.c_str(), MouseEvent, NULL);
    }
  }

  GCapture *GardzinCapture = new GCapture(nbrViews);

  //-----------------------------------------------------------------------------
  //-----------------------------------------------------------------------------
  // Inicjalizacja odejmowania tla, zmienne potrzebne do MOG2
  // wektor convexow tez sie przyda
  vector<Convex *> convex_vec;
  for (int i = 0; i < nbrViews; i++) {
    convex_vec.push_back(new Convex());
  }
  // bg->fTau = 0.000001;
  Mat fore;
  Mat frame;

  int x_size, y_size;
  MFrame = new marked_frame(640, 480);
  char _char;
  int waitTime = 1;
  DI_LOG("Entering main loop.", LogLevel::kDefault);
  Timer stoper;
  do {
    for (int c = 0; c < nbrViews; c++) {
      QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER *>(&start));
      GardzinCapture->QueryFrame(frame, fore, c);
      if (frame.empty()) {
        std::cout << "EMPTY FRAME!!" << std::endl;
        break;
      }
      _char = NULL;

      x_size = frame.cols;
      y_size = frame.rows;

      // bg_vec[c]->operator()(frame,fore);
      // imshow("asd",fore);
      threshold(fore, fore, current_threshold, 255, CV_THRESH_BINARY);

      /*char nameBuffer[100];
      sprintf(nameBuffer,"d:/fightparking3/%d.bmp",frameCounter);
      cvSaveImage(nameBuffer,frame);
      frameCounter++;*/

      if (convex_vec[c]->is_background_ok(frame.cols, frame.rows)) {
        if (mode == 5) convex_vec[c]->SHIELD((Mat)frame, fore, c);
      } else {
        std::cout << "!is_background_ok(frame.cols,frame.rows)" << std::endl;
      }
      // else
      //{
      //	delete bg_vec[c];//>~BackgroundSubtractorMOG2();
      //	bg_vec[c] = new BackgroundSubtractorMOG2(10000,
      // mogThreshold,true); 	cout<<"Background reset"<<endl;
      //}

      TrackObjects3D(frame, convex_vec, marker_coord, c);

      QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER *>(&stop));
      ms = (static_cast<long double>(stop) - start) / freq * 1000;
      if (mode != 7) imshow(pathDir[c], frame);

      _char = cvWaitKey(waitTime);
      if (_char == 's') {
        char nameBuffer[100];
        sprintf(nameBuffer, "d:/ramkiTest/%d.bmp", frameCounter);
        imwrite(nameBuffer, frame);
        frameCounter++;
      }
      if (_char == 'p')
        if (waitTime == 1)
          waitTime = 0;
        else
          waitTime = 1;
    }
  } while (_char != 'q' && !frame.empty());
  if (_char == 'q') DI_LOG("Quit by user (<q> pressed).", LogLevel::kDefault);
  if (frame.empty()) DI_LOG("End of stream reached.", LogLevel::kDefault);
  stoper.PrintElapsed("Finished after");
  for (auto img : Collector::getInstance().detections)
    imwrite(img.first, img.second);
  stoper.PrintElapsed("saving detections");
  DI_LOG("Leaving main loop. Calling destructors...", LogLevel::kDefault);
  cv::destroyAllWindows();
  std::cout << std::endl;

  delete GardzinCapture;
}

void match_objects(vector<vector<Point2f>> obj_positions,
                   vector<vector<double>> &matches, int view,
                   vector<double> path) {
  for (int i = 0; i < (int)obj_positions[view].size(); i++) {
    path.push_back(i);
    if (view < (int)obj_positions.size() - 1) {
      match_objects(obj_positions, matches, view + 1, path);
    } else {
      float sumx = 0;  //œrodek masy
      float sumy = 0;
      for (int c = 0; c < (int)path.size(); c++) {
        sumx += obj_positions[c][(int)path[c]].x;
        sumy += obj_positions[c][(int)path[c]].y;
      }
      sumx /= path.size();
      sumy /= path.size();

      double dist = 0;
      dist = GetDist(obj_positions[0][(int)path[0]], Point2f(sumx, sumy));
      path.push_back(dist);
      matches.push_back(path);
      path.erase(path.begin() + path.size() - 1);
      //	break;
    }
    path.erase(path.begin() + path.size() - 1);
  }
}

void sort_matches(vector<vector<double>> &matches) {
  int n = matches.size();
  do {
    for (int i = 0; i < n - 1; i++) {
      if (matches[i][matches[i].size() - 1] >
          matches[i + 1][matches[i].size() - 1]) {
        matches[i].swap(matches[i + 1]);
      }
    }
    n--;
  } while (n > 1);
}

void filter_matches(vector<vector<double>> &matches, double dist) {
  for (int i = 0; i < (int)matches.size(); i++) {
    if (matches[i][matches[i].size() - 1] > dist) {
      matches.erase(matches.begin() + i);
      i--;
    }
  }
}

void label_objects(vector<vector<double>> &matches, vector<Convex *> blob_vec) {
  static int object_id = 1;
  for (int i = 0; i < (int)matches.size(); i++) {
    int is_labeled = 0;
    for (int j = 0; j < (int)matches[i].size() - 1; j++) {
      if (blob_vec[j]->detected_objects[(size_t)matches[i][j]]->number == -1) {
        is_labeled++;
      }
    }
    if (is_labeled == matches[i].size() - 1) {
      for (int j = 0; j < (int)matches[i].size() - 1; j++) {
        blob_vec[j]->detected_objects[(size_t)matches[i][j]]->number =
            object_id;
        blob_vec[j]->detected_objects[(size_t)matches[i][j]]->SetObjNumber();
      }
      object_id++;

      for (int l = i + 1; l < (int)matches.size(); l++) {
        for (int c = 0; c < (int)matches[l].size() - 1; c++) {
          if (matches[i][c] == matches[l][c]) {
            matches.erase(matches.begin() + l);
            l--;
            break;
          }
        }
      }

    } else  // if(is_labeled > 0)
    {
      int label = 1000000;

      for (int j = 0; j < (int)matches[i].size() - 1; j++) {
        if (blob_vec[j]->detected_objects[(size_t)matches[i][j]]->number <
                label &&
            blob_vec[j]->detected_objects[(size_t)matches[i][j]]->number > -1) {
          label = blob_vec[j]->detected_objects[(size_t)matches[i][j]]->number;
        }
      }

      for (int j = 0; j < (int)matches[i].size() - 1; j++)
        blob_vec[j]->detected_objects[(size_t)matches[i][j]]->number = label;

      for (int l = i + 1; l < (int)matches.size(); l++) {
        for (int c = 0; c < (int)matches[l].size() - 1; c++) {
          if (matches[i][c] == matches[l][c]) {
            matches.erase(matches.begin() + l);
            l--;
            break;
          }
        }
      }
    }
    // else
    //{
    //	for(int l = i+1; l < matches.size(); l++)
    //	{
    //		for(int c = 0; c < matches[l].size()-1; c++)
    //		{
    //			if(matches[i][c] == matches[l][c])
    //			{
    //				matches.erase(matches.begin()+l);
    //				l--;
    //				break;
    //			}
    //		}
    //	}
    //}
  }
}

void match_text(vector<vector<double>> &matches, vector<Convex *> blob_vec,
                int a, int b) {
  for (int i = 0; i < (int)blob_vec[a]->detected_objects.size(); i++) {
    for (int j = 0; j < (int)blob_vec[b]->detected_objects.size(); j++) {
      if (blob_vec[b]->detected_objects[j]->number > -1 &&
          blob_vec[a]->detected_objects[i]->number > -1) {
        if (blob_vec[a]->detected_objects[i]->number ==
            blob_vec[b]->detected_objects[j]->number) {
          for (int beh = 0;
               beh < (int)blob_vec[a]->detected_objects[i]->bFilter.size();
               beh++) {
            if (blob_vec[a]->detected_objects[i]->bFilter[beh]->found == true ||
                blob_vec[b]->detected_objects[j]->bFilter[beh]->found == true) {
              blob_vec[a]->detected_objects[i]->bFilter[beh]->found = true;
              blob_vec[b]->detected_objects[j]->bFilter[beh]->found = true;
            }
          }
        }
      }
    }
  }
}

void DiffImage::TrackObjects3D(Mat Frame, vector<Convex *> blob_vec,
                               vector<vector<Point2f>> &marker_coord,
                               int view) {
  static vector<Mat> H;
  static vector<Mat> iH;
  static bool first_run = true;
  //	static int object_id = 1;

  vector<vector<Point2f>> obj_positions;

  if (first_run) {
    vector<Point2f> obj_corners(
        4);  // wspó³rzêdne zewnêtrznych wierzcho³ków markera z pliku!!!
    obj_corners[0] = cvPoint(0, 0);
    obj_corners[1] = cvPoint(370, 0);
    obj_corners[2] = cvPoint(370, 370);
    obj_corners[3] = cvPoint(0, 370);

    for (int i = 0; i < (int)blob_vec.size(); i++) {
      H.push_back(getPerspectiveTransform(marker_coord[i], obj_corners));
      iH.push_back(getPerspectiveTransform(obj_corners, marker_coord[i]));
    }
    first_run = false;
  }

  for (int c = 0; c < (int)blob_vec.size(); c++) {
    vector<Point2f> temp_positions;
    vector<Point2f> orginal_positions;
    for (int i = 0; i < (int)blob_vec[c]->detected_objects.size(); i++) {
      orginal_positions.push_back(
          (Point2f)blob_vec[c]->detected_objects[i]->KFilter->measPt);
    }
    if (orginal_positions.size() > 0)
      perspectiveTransform(orginal_positions, temp_positions, H[c]);
    obj_positions.push_back(temp_positions);
  }

  ////////////////////////////////////////////////////////////////////////////////////obj
  /// matching

  vector<vector<double>> matches;
  vector<double> path;
  bool match_possible = true;
  for (int i = 0; i < (int)obj_positions.size(); i++) {
    if (obj_positions[i].size() == 0) match_possible = false;
  }

  if (match_possible) {
    match_objects(obj_positions, matches, 0, path);
    sort_matches(matches);
    filter_matches(matches, 400);
    label_objects(matches, blob_vec);
    //////////////////////////////////////////////////////////usuwanie tych
    /// samych id
    for (int j = 0; j < (int)matches.size(); j++) {
      for (int i = 0; i < (int)matches[j].size() - 1; i++) {
        view = i;
        int label =
            blob_vec[view]->detected_objects[(size_t)matches[j][view]]->number;
        for (int o = 0; o < (int)blob_vec[view]->detected_objects.size(); o++) {
          if (label == blob_vec[view]->detected_objects[o]->number &&
              o != matches[j][view]) {
            blob_vec[view]->detected_objects[o]->number = -1;
          }
        }
      }
    }
    //////////////////////////////////////////////////////////
    for (int i = 0; i < (int)matches.size(); i++) {
      vector<bool> k_filter;
      k_filter.resize(
          blob_vec[0]->detected_objects[(size_t)matches[0][0]]->bFilter.size(),
          false);

      for (int j = 0; j < (int)matches[i].size() - 1; j++) {
        for (int k = 0; k < (int)blob_vec[j]
                                ->detected_objects[(size_t)matches[i][j]]
                                ->bFilter.size();
             k++) {
          if (blob_vec[j]
                  ->detected_objects[(size_t)matches[i][j]]
                  ->bFilter[k]
                  ->found == true) {
            k_filter[k] = true;
          }
        }
      }

      for (int j = 0; j < (int)matches[i].size() - 1; j++) {
        for (int k = 0; k < (int)blob_vec[j]
                                ->detected_objects[(size_t)matches[i][j]]
                                ->bFilter.size();
             k++) {
          blob_vec[j]
              ->detected_objects[(size_t)matches[i][j]]
              ->bFilter[k]
              ->found = k_filter[k];
        }
      }
    }
    //////////////////////////////////////////////////////////////////////////////////////////////przepisywanie
    /// is inside
    for (int i = 0; i < (int)matches.size(); i++) {
      bool is_inside = false;
      for (int j = 0; j < (int)matches[i].size() - 1; j++) {
        if (blob_vec[j]
                ->detected_objects[(size_t)matches[i][j]]
                ->is_inside_restricted_area == true) {
          is_inside = true;
          break;
        }
      }

      if (is_inside) {
        for (int j = 0; j < (int)matches[i].size() - 1; j++) {
          blob_vec[j]
              ->detected_objects[(size_t)matches[i][j]]
              ->is_inside_restricted_area = true;
        }
      }
    }
    ///////////////////////////////////////////////////////////////////////////////////////////////////////przepisywanie
    /// tekstu gdy w mniej niz N widokach

    /////////////////////////////////////////////////////////////////////////////////////////
  }
  for (int i = 0; i < (int)blob_vec.size(); i++) {
    for (int j = 0; j < (int)blob_vec.size(); j++) {
      if (i != j) match_text(matches, blob_vec, i, j);
    }
  }
}

void DiffImage::RestrictedAreaCheck(IplImage *frame, vector<Point2f> &vec,
                                    int camera, vector<Mat> H_f,
                                    vector<vector<Point2f>> &marker_coord,
                                    vector<Convex *> blob_vec) {
  int outside_thresh = 3;

  static vector<Mat> H;
  static vector<Mat> iH;
  static bool first_run = true;
  static int object_id = 1;

  if (first_run) {
    vector<Point2f> obj_corners(
        4);  // wspó³rzêdne zewnêtrznych wierzcho³ków markera z pliku!!!
    obj_corners[0] = cvPoint(0, 0);
    obj_corners[1] = cvPoint(370, 0);
    obj_corners[2] = cvPoint(370, 370);
    obj_corners[3] = cvPoint(0, 370);

    for (int i = 0; i < (int)marker_coord.size(); i++) {
      H.push_back(getPerspectiveTransform(marker_coord[i], obj_corners));
      iH.push_back(getPerspectiveTransform(obj_corners, marker_coord[i]));
    }
    first_run = false;
  }

  if (camera == 0) {
    MFrame->draw_circles(frame, vec);
    if ((int)vec.size() < 2 && (int)vec.size() != MFrame->points_counter)
      MFrame->points_counter = (int)vec.size();
    if (vec.size() > 1) {
      MFrame->points_counter = (int)vec.size();
      MFrame->draw_shape(frame, vec);
    }
    if (MFrame->shape_closed) {
      for (int i = 0; i < (int)blob_vec[camera]->detected_objects.size(); i++) {
        if (!blob_vec[camera]->detected_objects[i]->human &&
            (int)blob_vec[camera]->detected_objects[i]->KFilter->posv.size() >
                5) {
          if (MFrame->is_inside(
                  blob_vec[camera]->detected_objects[i],
                  vec))  // ||
                         // blob_vec[camera]->detected_objects[i]->is_inside_restricted_area)
          {
            blob_vec[camera]->detected_objects[i]->outside_counter = 0;
            //	rectangle((Mat)frame,blob_vec[camera]->detected_objects[i]->rect,cvScalar(0,255,255),2);
            //	if(!blob->detected_objects[i]->is_inside_restricted_area)
            //	{
            //	cout << '\a' << endl << "\b";
            blob_vec[camera]->detected_objects[i]->is_inside_restricted_area =
                true;
            blob_vec[camera]->detected_objects[i]->inside_counter++;
            //	}
          } else {
            if (blob_vec[camera]->detected_objects[i]->outside_counter++ >
                outside_thresh) {
              blob_vec[camera]->detected_objects[i]->is_inside_restricted_area =
                  false;
              blob_vec[camera]->detected_objects[i]->inside_counter = 0;
            }
          }
        }
      }
    }
  } else {
    if (vec.size() > 0) {
      vector<Point2f> positions(vec.size());
      perspectiveTransform(vec, positions, H[0]);
      vector<Point2f> final(vec.size());
      perspectiveTransform(positions, final, iH[camera]);
      MFrame->draw_circles(frame, final);
      if (vec.size() > 1) {
        MFrame->draw_shape(frame, final);
      }
      if (MFrame->shape_closed) {
        for (int i = 0; i < (int)blob_vec[camera]->detected_objects.size();
             i++) {
          if (!blob_vec[camera]->detected_objects[i]->human &&
              (int)blob_vec[camera]->detected_objects[i]->KFilter->posv.size() >
                  5) {
            if (MFrame->is_inside(
                    blob_vec[camera]->detected_objects[i],
                    final))  //||
                             // blob_vec[camera]->detected_objects[i]->is_inside_restricted_area)
            {
              blob_vec[camera]->detected_objects[i]->outside_counter = 0;
              //		rectangle((Mat)frame,blob_vec[camera]->detected_objects[i]->rect,cvScalar(0,255,255),2);
              //	if(!blob->detected_objects[i]->is_inside_restricted_area)
              //	{
              //	cout << '\a' << endl << "\b";
              blob_vec[camera]->detected_objects[i]->is_inside_restricted_area =
                  true;
              blob_vec[camera]->detected_objects[i]->inside_counter++;
              //	}
            } else {
              if (blob_vec[camera]->detected_objects[i]->outside_counter++ >
                  outside_thresh) {
                blob_vec[camera]
                    ->detected_objects[i]
                    ->is_inside_restricted_area = false;
                blob_vec[camera]->detected_objects[i]->inside_counter = 0;
              }
            }
          }
        }
      }
    }
  }
}
}  // namespace bd
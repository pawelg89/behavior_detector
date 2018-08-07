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
#include "..\includes\Convex.h"
#include "..\includes\GCapture.h"
#include "..\includes\collector.h"
#include "..\includes\logger.h"
#include "..\includes\marked_frame.h"
#include "..\includes\timer.h"
#include "..\includes\track_objects.h"

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
  Collector::getInstance().SaveData();
  stoper.PrintElapsed("saving collected data");
  DI_LOG("Leaving main loop. Calling destructors...", LogLevel::kDefault);
  cv::destroyAllWindows();
  std::cout << std::endl;

  delete GardzinCapture;
}

}  // namespace bd
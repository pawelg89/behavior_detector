#include "..\includes\GCapture.h"
// STL includes
#include <fstream>
#include <iomanip>
#include <string>
#include <strstream>
#include <thread>
// Project includes
#include "..\..\stdafx.h"
#include "..\includes\logger.h"
#include "..\includes\signaler.h"

namespace bd {
//---------------------------------------------------------------------------
//-----------UWAGA STREAFA ZAGROZONA, ZMIENNE GLOBALNE-----------------------
//----------------------NIE DOTYKAC!!!!--------------------------------------
//---------------------------------------------------------------------------
std::vector<CRITICAL_SECTION> critical;
std::vector<HANDLE> eventStart;
std::vector<cv::Mat> buffer;
std::vector<cv::Mat> fore_vec;
std::vector<cv::BackgroundSubtractorMOG2 *> bg_vec;
std::vector<int> gc_frames_count;
std::vector<cv::VideoCapture *> video{};
std::vector<std::ifstream> fore_lists;
std::vector<std::ifstream> frame_lists;
bool kAsynchronous;
std::atomic<bool> finish_requested = false;

namespace {
bool IsConnectedCamera(const std::string &path) {
  for (size_t i = 0; i < 10; ++i)
    if (path == std::to_string(i)) return true;
  return false;
}
bool IsStreamCamera(const std::string &path) {
  if (path.find("rtsp") != std::string::npos) return true;
  if (path.find("http") != std::string::npos) return true;
  return false;
}
bool IsSupportedImage(const std::string &path) {
  if (path.find(".png") != std::string::npos) return true;
  if (path.find(".jpg") != std::string::npos) return true;
  return false;
}
void SendMockFrame(int cameraNumber) {
  std::string next_image = "closing_mock_frame";
  cv::Mat mock_frame{cv::Size(640, 480), CV_8UC1, cv::Scalar(0)};
  size_t wait_counter = 0;
  while (!TryEnterCriticalSection(&critical[cameraNumber])) {
    ++wait_counter;
  }
  if (wait_counter > 0) {
    std::string msg =
        "TryEnterCriticalSection calls=" + std::to_string(wait_counter) +
        " cam=" + std::to_string(cameraNumber);
    LOG("image_thread", msg, LogLevel::kMega);
  }
  LOG("image_thread", "sending mock frame (entirely black)", LogLevel::kSetup);
  bd::Signaler::getInstance().SignalPathChange(next_image, "reset_tracker");
 // buffer[cameraNumber] = mock_frame;
  fore_vec[cameraNumber] = mock_frame;
  LeaveCriticalSection(&critical[cameraNumber]);
}
}  // namespace

unsigned int __stdcall buffer_thread(void *arg) {
  int cameraNumber = (int)arg;
  std::cout << video.size() << " <--> " << cameraNumber << std::endl;
  while ((int)video.size() < cameraNumber + 1) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    std::cout << "FAAK" << std::endl;
  }
  while (!finish_requested) {
    while (!TryEnterCriticalSection(&critical[cameraNumber])) {
    }
    if (!kAsynchronous && gc_frames_count[cameraNumber] > 0) {
      LeaveCriticalSection(&critical[cameraNumber]);
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
      continue;
    }
    // video[cameraNumber]->grab();
    video[cameraNumber]->grab();
    video[cameraNumber]->retrieve(buffer[cameraNumber]);
    bg_vec[cameraNumber]->operator()(buffer[cameraNumber],
                                     fore_vec[cameraNumber]);
    SetEvent(eventStart[cameraNumber]);
    ++gc_frames_count[cameraNumber];

    LeaveCriticalSection(&critical[cameraNumber]);
  }

  // cvReleaseCapture(&video);
  video[cameraNumber]->release();
  std::cout << "video[cameraNumber]->release(); DONE" << std::endl;
  return 0;
}

unsigned int __stdcall image_thread(void *arg) {
  int cameraNumber = (int)arg;
  std::cout << fore_lists.size() << " <--> " << cameraNumber << std::endl;
  while ((int)fore_lists.size() < cameraNumber + 1) {
    std::this_thread::sleep_for(std::chrono::seconds(1));
    std::cout << "fore_lists.size() < cameraNumber + 1" << std::endl;
  }
  std::string next_image;
  while (!finish_requested && !fore_lists[cameraNumber].eof() &&
         !frame_lists[cameraNumber].eof()) {
    size_t wait_counter = 0;
    while (!TryEnterCriticalSection(&critical[cameraNumber])) {
      ++wait_counter;
    }
    if (!kAsynchronous && gc_frames_count[cameraNumber] > 0) {
      LeaveCriticalSection(&critical[cameraNumber]);
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
      continue;
    }
    if(wait_counter > 0) {
      std::string msg =
          "TryEnterCriticalSection calls=" + std::to_string(wait_counter) +
          " cam=" + std::to_string(cameraNumber);
      LOG("image_thread", msg, LogLevel::kMega);
    }
    
    frame_lists[cameraNumber] >> next_image;
    bd::Signaler::getInstance().SignalPathChange(next_image, "reset_tracker");
    buffer[cameraNumber] = cv::imread(next_image, CV_LOAD_IMAGE_COLOR);
    fore_lists[cameraNumber] >> next_image;
    fore_vec[cameraNumber] = cv::imread(next_image, CV_LOAD_IMAGE_GRAYSCALE);

    SetEvent(eventStart[cameraNumber]);
    ++gc_frames_count[cameraNumber];

    LeaveCriticalSection(&critical[cameraNumber]);
  }
  
  SendMockFrame(cameraNumber);
  LOG("image_thread", "camera[" + std::to_string(cameraNumber) + "] finished",
      LogLevel::kSetup);
  return 0;
}
//---------------------------------------------------------------------------
//-------------------KONIEC STREFY-------------------------------------------
//---------------------------------------------------------------------------

GCapture::GCapture(void) { finish_requested = false; }

GCapture::GCapture(int cams_number, bool async) {
  finish_requested = false;
  buffer.resize(cams_number);
  fore_vec.resize(cams_number);
  critical.resize(cams_number);
  eventStart.resize(cams_number);
  fore_lists.resize(cams_number);
  frame_lists.resize(cams_number);
  gc_frames_count.resize(cams_number, 0);
  for (int i = 0; i < cams_number; i++) {
    // Ustalenie sciezek dostepu do strumieni wideo
    char pathDirX[1000] = "";
    std::ostrstream monkeys(pathDirX, (int)sizeof(pathDirX),
                            std::ios::app);  // hehe legacy stream name ];->
    std::string tempDir;
    monkeys.seekp(0);
    monkeys << "pathDir" << i << std::ends;
    load_data("parameters.txt", pathDirX, tempDir);
    // Inicjalizacja sekcji krytycznych dla poszczegolnych kamer i eventu na
    // start mozliwosci odczytania
    InitializeCriticalSection(&critical[i]);
    eventStart[i] = CreateEvent(0, 0, 0, 0);

    if (IsImageSequence(tempDir)) 
      InitializeImages(tempDir, i);
    else
      InitializeVideo(tempDir, i);
  }
}

void GCapture::InitializeVideo(const std::string &path, int i) {
  // MOGI !!!
  float mogThreshold;
  float backgroundRatio;
  if (!load_data("parameters.txt", "mogThreshold", mogThreshold))
    mogThreshold = 50.0f;
  if (!load_data("parameters.txt", "backgroundRatio", backgroundRatio))
    backgroundRatio = 0.7f;
  bg_vec.push_back(new cv::BackgroundSubtractorMOG2(10000, mogThreshold, true));
  //		bg_vec[i]->backgroundRatio = backgroundRatio;

  // Wczytanie strumieni do handlerow no i zapisanie tych sciezek na potem (w
  // sumie nie wiem czy sie przyda ale moze sie przyda :P)
  std::cout << "Video path: " << path << std::endl;
  video.clear();
  video.push_back(new cv::VideoCapture(path));
  pathDir_.push_back(path);

  kAsynchronous = IsLiveCamera(path);

  // Wczytanie watkow i ich uruchomienie
  HANDLE thrdBuffer =
      (HANDLE)_beginthreadex(0, 0, &buffer_thread, (void *)i, 0, 0);
  camThreads_.push_back(thrdBuffer);
  WaitForSingleObject(eventStart[i], INFINITE);
  std::cout << "Camera[" << i << "] connection established." << std::endl;
}

void GCapture::InitializeImages(const std::string &path, int i) {
  // Wczytanie strumieni do handlerow no i zapisanie tych sciezek na potem (w
  // sumie nie wiem czy sie przyda ale moze sie przyda :P)
  std::cout << "Images list file: " << path << std::endl;
  pathDir_.push_back(path);

  kAsynchronous = false;
  fore_lists[i].open(path);
  frame_lists[i].open(GetFramesPath(path, i));
  // Wczytanie watkow i ich uruchomienie
  HANDLE thrdBuffer =
      (HANDLE)_beginthreadex(0, 0, &image_thread, (void *)i, 0, 0);
  camThreads_.push_back(thrdBuffer);
  WaitForSingleObject(eventStart[i], INFINITE);
  std::cout << "Image sequence[" << i << "] loaded." << std::endl;
}

bool GCapture::IsLiveCamera(const std::string &path) {
  if (IsConnectedCamera(path)) return true;
  if (IsStreamCamera(path)) return true;
  return false;
}

bool GCapture::IsImageSequence(const std::string &path) {
  if (path.find(".txt") != std::string::npos) {
    std::ifstream img_list(path);
    if (!img_list.is_open())
      throw std::invalid_argument("Couldn't open '" + path + "'.");
    std::string img_path;
    img_list >> img_path;
    img_list.close();
    if (IsSupportedImage(img_path)) return true;
  }
  return false;
}

GCapture::~GCapture(void) {
  // Usuwanie watkow
  std::cout << "Closing GCapture..." << std::endl;
  finish_requested = true;
  for (int i = 0; i < (int)camThreads_.size(); i++) {
    TerminateThread(camThreads_[i], 0);
    CloseHandle(camThreads_[i]);
    CloseHandle(eventStart[i]);
  }
  critical.clear();
  eventStart.clear();
  buffer.clear();
  fore_vec.clear();
  for (auto bg : bg_vec) delete bg;
  bg_vec.clear();
  gc_frames_count.clear();
  for (auto vid : video) delete vid;
  video.clear();
  for (auto &stream : fore_lists) stream.close();
  for (auto &stream : frame_lists) stream.close();
  std::cout << "GCapture deleted successfully." << std::endl;
}

void GCapture::QueryFrame(cv::Mat &frame, cv::Mat &fore, int i) {
  if (kAsynchronous) AsyncQuery(frame, fore, i);
  else               SyncQuery(frame, fore, i);
}

void GCapture::AsyncQuery(cv::Mat &frame, cv::Mat &fore, int i) {
  while (!TryEnterCriticalSection(&critical[i])) {
  }
  if (!buffer[i].empty() && gc_frames_count[i] > 0) {
    buffer[i].copyTo(frame);
    fore_vec[i].copyTo(fore);
    if (--gc_frames_count[i] > 0) {
      std::string msg = "Camera[" + std::to_string(i);
      msg += "] dropped frames: " + std::to_string(gc_frames_count[i]);
      LOG("GCapture", msg, LogLevel::kMega);
      gc_frames_count[i] = 0;
    }
  } else {
    frame = cv::Mat{};
  }
  LeaveCriticalSection(&critical[i]);
}

void GCapture::SyncQuery(cv::Mat &frame, cv::Mat &fore, int i) {
  bool finished = false;
  unsigned short skips = 10;
  while (!finished) {
    while (!TryEnterCriticalSection(&critical[i])) {
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
      LOG("SyncQuery", "->TryEnterCriticalSection", LogLevel::kMega);
    }
    if (!buffer[i].empty() && gc_frames_count[i] > 0) {
      finished = true;
      buffer[i].copyTo(frame);
      fore_vec[i].copyTo(fore);
      if (--gc_frames_count[i] > 0) {
        std::string msg = "Camera[" + std::to_string(i);
        msg += "] dropped frames: " + std::to_string(gc_frames_count[i]);
        LOG("SyncQuery", msg, LogLevel::kWarning);
        gc_frames_count[i] = 0;
      }
    } else {
      if (skips-- > 0) {
        LeaveCriticalSection(&critical[i]);
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        continue;
      }
      finished = true;
      frame = cv::Mat{};
      LOG("SyncQuery", "FINISHED!!!", LogLevel::kWarning);
    }
  }  // while(!finished)
  LeaveCriticalSection(&critical[i]);
}

std::string GCapture::GetFramesPath(const std::string &path, int i) {
  auto pos = path.find(".txt");
  if (pos != std::string::npos) {
    auto substr = path.substr(0, pos);
    return std::string(substr + "_frames.txt");
  }
  throw std::runtime_error("GetFramesPath provided with non .txt file.");
}

}  // namespace bd
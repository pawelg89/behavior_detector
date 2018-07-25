// STL includes
#include <fstream>
#include <strstream>
// OpenCV
#include <opencv\cv.h>
#include <opencv2\highgui\highgui.hpp>
// Project includes
#include "..\includes\DiffImage.h"
#include "..\includes\logger.h"

int log_level;
constexpr const char* kEntryMessage =  "------------ ENTRY POINT ------------";
constexpr const char* kFinishMessage = "--------- PROGRAM FINISHED ----------";
int VR_LOG(std::string msg, bd::LogLevel level) {
  return bd::LOG("main", msg, level);
}

int main(int argc, char* argv[]) {
  if (argc > 1) {
    std::ifstream file;
    file.open(argv[1], std::ios_base::in);
    if (!file.is_open()) {
      std::cout << "Video file does not exist." << std::endl;
      return 0;
    }
    file.close();
    bd::DiffImage* dif = new bd::DiffImage(argv[1]);
    dif->DiffImageAction();
  } else {
    VR_LOG(kEntryMessage, bd::LogLevel::kSetup);
    bd::DiffImage* dif = new bd::DiffImage(
        "C:/Users/Martyna/Videos/sekwencje testowe/machanie_test.avi");

    try {
      dif->DiffImageAction2();
    } catch (std::string err_msg) {
      std::cout << err_msg << std::endl;
      VR_LOG(err_msg, bd::LogLevel::kError);
      system("pause");
    } catch (std::ios_base::failure err) {
      std::cout << "Brak pozwolenia na zapis na dysku." << std::endl;
      VR_LOG("Brak pozwolenia na zapis na dysku.", bd::LogLevel::kError);
    }
    VR_LOG(kFinishMessage, bd::LogLevel::kSetup);
  }

  // system("pause");
  return 0;
}

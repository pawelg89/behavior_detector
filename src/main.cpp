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
constexpr const char* kInputFilePath = "C:/Users/Martyna/Videos/sekwencje testowe/machanie_test.avi";
int VR_LOG(std::string msg, bd::LogLevel level) {
  return bd::LOG("main", msg, level);
}

void SingleRun(const std::string &input_file = kInputFilePath) {
  VR_LOG(kEntryMessage, bd::LogLevel::kSetup);
  bd::DiffImage* dif = new bd::DiffImage(input_file);

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
  delete dif;
}

int main(int argc, char* argv[]) {
  size_t repetitions = 1;
  bd::load_data("parameters.txt", "repetitions", repetitions);
  for (size_t i = 0; i < repetitions; ++i) {
    SingleRun();
  }

  // system("pause");
  return 0;
}

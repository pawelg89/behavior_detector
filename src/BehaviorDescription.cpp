#include "..\includes\BehaviorDescription.h"
// STL
#include <fstream>
// Project includes
#include "../includes/logger.h"

namespace bd {

namespace {
constexpr const char *kBehaviorsListPath = "behavior_names_list.txt";
}

BehaviorDescription::BehaviorDescription(void) {
  behavior_types_.reserve(7);
  behavior_types_.emplace_back(-1, "unknown");
  behavior_types_.emplace_back(2, "help");
  behavior_types_.emplace_back(3, "faint");
  behavior_types_.emplace_back(4, "fight");
  behavior_types_.emplace_back(5, "fall");
  behavior_types_.emplace_back(6, "pain");
  behavior_types_.emplace_back(7, "kneel");
  LoadBehaviorList();
}

BehaviorDescription::~BehaviorDescription(void) {}

void BehaviorDescription::SaveBehaviorDescriptor(int id, const std::string &name) {
  bool skip_save; BehaviorType beh_type;
  std::tie(skip_save, beh_type) = GetBehaviorType(id, name);
  if (skip_save) return;
  //BehaviorType beh_type = std::move(loaded_beh.second);
  int cntr = ReadCounter(beh_type.cntr_file);
  std::string descr_name = MakeDescriptorName(beh_type.name, cntr);

  std::ofstream descriptor_file;
  descriptor_file.open(descr_name, std::ios_base::binary);
  bool temp = descriptor_file.is_open();
  int size1 = descriptor.size();  // Liczba polaczen
  int size3 = size1 + 1;

  // header
  descriptor_file.write((char*)&size1, 4);
  descriptor_file.write((char*)&beh_type.id, 4);

  // int size2 = descriptor[size1-1].size();//Liczba punktow deskryptora dla
  // kazdego stanu  descriptor_file.write((char*)&size2,4);

  // Wektor z rozmiarami deskryptorów
  descriptor_file.write((char*)sizes, sizeof(int) * size1);

  // Progi dla wszystkich stanow
  double *thresholds = new double[size1];
  // cout<<"Give threshold: "<<endl;
  double temp1;
  temp1 = 0.16;  // cin>>temp1;
  for (int i = 0; i < size1; i++) thresholds[i] = temp1;

  descriptor_file.write((char*)thresholds, sizeof(double) * size1);

  // Mapa polaczen miedzy stanami, przy tworzeniu domyslna tylko z ³ancuchem
  // 1->2->3-> .. ->n
  map = new bool[size3 * size3];
  for (int i = 0; i < size3; i++)
    for (int j = 0; j < size3; j++) map[i * size3 + j] = (j == i + 1);

  descriptor_file.write((char*)map, sizeof(bool) * size3 * size3);

  // Oznaczenia czy sa to ostatnie stany
  bool* isLast = new bool[size3];
  for (int i = 0; i < size3; i++) {
    isLast[i] = ((i == size1) ? true : false);
  }
  descriptor_file.write((char*)isLast, sizeof(bool) * size3);

  // tablica z punktami (x,y) dla deskryptorow. <serialized>
  for (int i = 0; i < size1; i++) {
    for (int j = 0; j < sizes[i]; j++) {
      descriptor_file.write((char*)&descriptor[i][j].x, sizeof(double));
      descriptor_file.write((char*)&descriptor[i][j].y, sizeof(double));
    }
  }
  descriptor_file.close();

  // Update Numeracji deskryptorów w pliku
  SaveCounter(beh_type.cntr_file, ++cntr);

  delete[] thresholds;
  delete[] map;
  delete[] isLast;
}

void BehaviorDescription::SaveCounter(const std::string &file_name, int counter) {
  std::ofstream file;
  file.open(file_name);
  file << counter;
  file.close();
  LOG("BehaviorDescription", 
      std::string("Updating " + file_name + " counter to " + std::to_string(counter)), 
      LogLevel::kDetailed);
}

int BehaviorDescription::ReadCounter(const std::string &file_name) {
  std::ifstream file(file_name);
  int counter = 0;
  if (file.is_open()) {
    file >> counter;
    file.close();
    LOG("BehaviorDescription", 
        std::string("Reading from " + file_name + " counter is: " + std::to_string(counter)), 
        LogLevel::kDetailed);
  }
  return counter;
}

inline std::string BehaviorDescription::MakeDescriptorName(const std::string &behavior_name, int counter) {
  return std::string(behavior_name + "Descr" + std::to_string(counter) + ".dat");
}

BehaviorType BehaviorDescription::FindBehavior(int id) {
  for (const auto &behavior : behavior_types_) {
    if (behavior.id == id) return behavior;
  }
  return std::move(BehaviorType(-1, "unknown"));
}

BehaviorType BehaviorDescription::FindBehavior(const std::string &behavior_name) {
  for (const auto &behavior : behavior_types_) {
    if (behavior.name == behavior_name) return behavior;
  }
  return std::move(BehaviorType(-1, "unknown"));
}

int TryConvert(std::string input) {
  try {
    int id = std::atoi(input.c_str());
    return id;
  } catch (...) {
    std::string msg = "Probable syntax error in " + std::string(kBehaviorsListPath);
    LOG("BehaviorDescription", msg, LogLevel::kCritical);
  }
}

void BehaviorDescription::LoadBehaviorList() {
  std::ifstream file(kBehaviorsListPath);
  if (!file.is_open()) return;
  while (!file.eof()) {
   std::string name, id_s;
    file >> id_s;
    file >> name;
    int id = TryConvert(std::move(id_s));
    if (FindBehavior(id).id != -1 || FindBehavior(name).id != -1) {
      LOG("BehaviorDescription",
          std::string("Error! Behavior with ID=" + std::to_string(id) +
                      " or NAME=" + name + " has already been loaded."),
          LogLevel::kError);
      throw std::runtime_error("A duplicate behavior has been found on list "
                               + std::string(kBehaviorsListPath));
    } else if (id != -1 && name.empty()) {
      LOG("BehaviorDescription",
          std::string("Error! Behavior with ID=" + std::to_string(id) +
                      " saved without paired name."),
          LogLevel::kError);
      throw std::runtime_error("An unpaired behavior ID left on list "
                               + std::string(kBehaviorsListPath));
    } else if (id != -1 && !name.empty()) {
      behavior_types_.emplace_back(id, name);
      LOG("BehaviorDescription",
        std::string("Adding behavior: ID=" + std::to_string(id) + " NAME=" + name),
        LogLevel::kDetailed);
    }
  }
  file.close();
}

bool IsEndOfFile(const std::ifstream &stream, const int calls) {
  if (stream.eof()) {
    if (calls == 0)
      return true;
    else if (calls % 2 == 1)
      throw std::runtime_error("Odd number of parameters in " +
                                std::string(kBehaviorsListPath));
  }
  return false;
}

void BehaviorDescription::AddBehaviorToList(BehaviorType beh_type) {
  std::ifstream file_in(kBehaviorsListPath);
  if (!file_in.is_open()) {
    file_in.open(kBehaviorsListPath, std::ios_base::app);
  }
  while (!file_in.eof()) {
    int id, c = 0; std::string name;
    file_in >> id;
    if (IsEndOfFile(file_in, c++)) break;
    file_in >> name;
    if (FindBehavior(id).id != -1 || FindBehavior(name).id != -1) {
      file_in.close();
      LOG("BehaviorDescription",
          std::string("Behavior: ID=" + std::to_string(id) +
                      " NAME=" + name + " is already on the list."),
          LogLevel::kDetailed);
      return;
    }
    c++;
  }
  file_in.close();
  std::ofstream file(kBehaviorsListPath, std::ios_base::app);
  file << beh_type.id << " " << beh_type.name << "\n";
  file.close();
}

std::pair<bool, BehaviorType>
BehaviorDescription::GetBehaviorType(int id, const std::string &name) {
  if (id == -1) {
    std::string msg = "What type of descriptor filter is it? ";
    for (const auto &beh : behavior_types_)
      msg += std::to_string(beh.id) + "-" + beh.name + " ";
    std::cout << msg << std::endl;
    std::cin >> id;
    if (id == -1) return std::move(std::pair<bool, BehaviorType>(true, BehaviorType()));
  }
  // 'id' is supposed to be a meaningful ID at this point
  BehaviorType beh_type = FindBehavior(id);
  if (beh_type.name == "unknown") {
    std::cout << "Give name of behavior with ID=" << id << std::endl;
    std::cin >> beh_type.name;
    beh_type.id = id;
    beh_type.UpdateCounterFile();
    SaveCounter(beh_type.cntr_file, 0);
    AddBehaviorToList(beh_type);
    behavior_types_.push_back(beh_type);
  } else {
    LOG("BehaviorDescription",
        "Loaded behavior: " + beh_type.name + ", id " + std::to_string(beh_type.id),
        LogLevel::kDetailed);
  }
  return std::move(std::pair<bool, BehaviorType>(false, beh_type));
}

}  // namespace bd
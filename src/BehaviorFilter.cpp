#include "..\includes\BehaviorFilter.h"
// STL includes
#include <fstream>
// Project includes
#include "..\includes\BehaviorDescription.h"
#include "..\includes\logger.h"

namespace bd {

namespace {
int BF_LOG(const std::string &msg, const LogLevel level, bool new_line = true) {
  return LOG("BehaviorFilter", msg, level, new_line);
}
int BF_LogOnce(const std::string &msg, const LogLevel level, bool new_line = true) {
  return LogOnce("BehaviorFilter", msg, level, new_line);
}
}  // namespace

BehaviorFilter::BehaviorFilter(void) {}

BehaviorFilter::BehaviorFilter(const std::string& path)
    : descriptor_path(path) {
  BehaviorDescription beh_descr;
  method.resize(beh_descr.GetBehaviorTypesCount(), 0);
  if (!load_data("parameters.txt", "lPkt", lPkt)) lPkt = 10;
  std::string message = "lPkt=" + std::to_string(lPkt);

  for (size_t i = 2; i < method.size(); ++i) {
    std::string beh_name = beh_descr.FindBehavior(i).name;
    if (!load_data("parameters.txt", "method_" + beh_name, method[i])) method[i] = 0;
    message += ", method_" + beh_name + "=" + std::to_string(method[i]);
  }

  // Reading descriptor from file
  std::ifstream input;
  input.open(path, std::ios_base::binary);
  if (!input.is_open())
    throw(std::string) "Couldn't open descriptor file, path: " + path;

  int header[2];  // 0-liczba stanow glownych, 1-typ zachowania
  input.read((char*)header, sizeof(header));
  this->behaviorType = header[1];
  message += ", behaviorType=" + std::to_string(behaviorType);

  // liczba punktow w stanach
  int* sizes = new int[header[0]];
  input.read((char*)sizes, sizeof(int) * header[0]);
  message += ", stateSizes={" + std::to_string(sizes[0]);
  for (int i = 1; i < header[0]; ++i)
    message += "," + std::to_string(sizes[i]);
  message += "}";

  // Progi akceptacji wektora z punktami deskryptora
  double* thresholds = new double[header[0]];
  input.read((char*)thresholds, sizeof(double) * header[0]);
  message += ", threshold=" + std::to_string(thresholds[0]);

  // Mapa polaczen miedzy stanami
  int statesNumber = header[0] + 1;
  bool* map = new bool[statesNumber * statesNumber];
  input.read((char*)map, sizeof(bool) * statesNumber * statesNumber);

  // Oznaczenia czy sa to ostatnie stany
  bool* islast = new bool[statesNumber];
  input.read((char*)islast, sizeof(bool) * statesNumber);

  // Liczba wszystkich punktów deskryptora, potrzebna do odczytania samego
  // deskryptora
  int header_1 = 0;
  for (int i = 0; i < header[0]; i++) header_1 += sizes[i];

  // tablica z punktami (x,y) dla deskryptorow. <serialized>
  double* buffer = new double[header_1 * 2];
  input.read((char*)buffer, sizeof(double) * header_1 * 2);

  std::vector<std::vector<PointNorm>> behaviorDescr;
  behaviorDescr.resize(header[0]);
  int licznik_punktow = 0;
  for (int i = 0; i < header[0]; i++) {
    for (int j = 0; j < sizes[i]; j++) {
      behaviorDescr[i].push_back(
          PointNorm(buffer[licznik_punktow], buffer[licznik_punktow + 1]));
      licznik_punktow += 2;
    }
  }

  // Utworz siatke stanow (automat)
  // first state just with a handle to next states, mostly idle state
  temp_StateHandles.reserve(header[0]);
  temp_StateHandles.push_back(
      new BehaviorState(thresholds[0], this->method[behaviorType], this->lPkt));
  temp_StateHandles[0]->sttNumber = 0;
  temp_StateHandles[0]->behType = behaviorType;
  temp_StateHandles[0]->descriptor_path = path;
  // temp_StateHandles[0]->SetIdxStop( (int)behaviorDescr[0].size() );
  for (int i = 0; i < header[0]; i++) {
    temp_StateHandles.push_back(
        new BehaviorState(behaviorDescr[i], islast[i + 1], thresholds[i],
                          this->method[behaviorType], this->lPkt));
    temp_StateHandles[i + 1]->sttNumber = i + 1;
    temp_StateHandles[i + 1]->behType = behaviorType;
    temp_StateHandles[i + 1]->descriptor_path = path;
  }

  // Create connections
  for (int i = 0; i < statesNumber; i++) {
    std::vector<BehaviorState*> temp_nxtStts;
    for (int j = 0; j < statesNumber; j++) {
      if (map[statesNumber * i + j]) {
        temp_nxtStts.push_back(temp_StateHandles[j]);
      }
    }
    temp_StateHandles[i]->SetNextStates(temp_nxtStts);
    temp_nxtStts.clear();
  }

  // Filter description. It shows up on blob with detected behavior, some of
  // them are hardcoded to keep legacy behavior decriptors compatibile and new
  // ones are defined in 'behavior_names_list.txt'.
  behaviorDescription = beh_descr.FindBehavior(behaviorType).name;

  message = "description=" + behaviorDescription + ", " + message;
  BF_LogOnce(message, LogLevel::kSetup);

  firstState = temp_StateHandles[0];
  currentState = firstState;
  found = false;

  // Clearing helper vector
  delete[] sizes;
  delete[] thresholds;
  delete[] map;
  delete[] islast;
  delete[] buffer;
}

BehaviorFilter::BehaviorFilter(const std::string &path, int gs)
    : descriptor_path(path)  {
  // Reading descriptor from file
  std::ifstream input;
  input.open(path, std::ios_base::binary);
  int header[3];  // 0-liczba polaczen miedzy stanami, 1-liczba punktow dla
                  // jednej pozy
  input.read((char*)header, sizeof(header));
  behaviorType = header[2];
  double threshold;
  input.read((char*)&threshold, sizeof(double));
  std::vector<std::vector<PointNorm>> behaviorDescr;
  behaviorDescr.resize(header[0]);
  // Mapa polaczen miedzy stanami
  int statesNumber = header[0] + 1;
  bool* map = new bool[statesNumber * statesNumber];
  input.read((char*)map, sizeof(bool) * statesNumber * statesNumber);
  // tablica z punktami (x,y) dla deskryptorow. <serialized>
  double* buffer = new double[header[0] * header[1] * 2];
  input.read((char*)buffer, sizeof(double) * header[0] * header[1] * 2);
  for (int i = 0; i < header[0]; i++) {
    for (int j = 0; j < header[1]; j++) {
      behaviorDescr[i].push_back(
          PointNorm(buffer[i * 2 * header[1] + 2 * j],
                    buffer[i * 2 * header[1] + 2 * j + 1]));
    }
  }
  // first state just with a handle to next states, mostly idle state
  temp_StateHandles.push_back(new BehaviorState(threshold));  
  temp_StateHandles[0]->sttNumber = 0;
  temp_StateHandles[0]->behType = behaviorType;
  temp_StateHandles[0]->descriptor_path = path;
  for (int i = 0; i < header[0]; i++) {
    if (i == (header[0] - 1))
      temp_StateHandles.push_back(
          new BehaviorState(behaviorDescr[i], true, threshold));
    else
      temp_StateHandles.push_back(
          new BehaviorState(behaviorDescr[i], false, threshold));
    temp_StateHandles[i + 1]->sttNumber = i + 1;
    temp_StateHandles[i + 1]->behType = behaviorType;
    temp_StateHandles[i + 1]->descriptor_path = path;
  }
  // Create connections
  for (int i = 0; i < statesNumber; i++) {
    std::vector<BehaviorState*> temp_nxtStts;
    for (int j = 0; j < statesNumber; j++) {
      if (map[statesNumber * i + j]) {
        temp_nxtStts.push_back(temp_StateHandles[j]);
      }
    }
    temp_StateHandles[i]->SetNextStates(temp_nxtStts);
    temp_nxtStts.clear();
  }
  // Filter description. It shows up on blob with detected behavior, some of
  // them are hardcoded to keep legacy behavior decriptors compatibile and new
  // ones are defined in 'behavior_names_list.txt'.
  BehaviorDescription beh_descr;
  behaviorDescription = beh_descr.FindBehavior(behaviorType).name;

  firstState = temp_StateHandles[0];
  currentState = firstState;
  found = false;
  delete[] map;
  delete[] buffer;
}

BehaviorFilter::~BehaviorFilter() {
  for (int i = 0; i < (int)temp_StateHandles.size(); i++) {
    delete temp_StateHandles[i];
  }
  temp_StateHandles.clear();
  method.clear();
}

void BehaviorFilter::Check(std::vector<PointNorm> input, bool isMoving) {
  currentState = currentState->ChangeState(input, isMoving);
  // Przypadek, gdy zbyt dlugo czekalismy na nastepny stan
  if (currentState == NULL) currentState = firstState;
  // Jesli przeszlismy wlasnie do ostatniego to juz to wykrywamy
  if (currentState->lastState) {
    std::string msg = behaviorDescription +
                      ", objNum: " + std::to_string(objNumber) +
                      ", from descriptor: " + descriptor_path;
    BF_LOG(msg, LogLevel::kDefault);
  }

  if (found || currentState->lastState) {
    currentState = firstState;
    found = true;
  } else {
    found = false;
  }
}

std::vector<int> BehaviorFilter::BehaviorType() {
  std::vector<int> result;
  if (found) {
    BehaviorDescription beh_descr;
    auto beh_type = beh_descr.FindBehavior(behaviorDescription);
    if (beh_type.name != "unknown") result.push_back(beh_type.id);
  }
  return result;
}

int BehaviorFilter::GetCurrentStateNumber() { return currentState->sttNumber; }

void BehaviorFilter::Rewrite(char* path, double thresholdNew) {
  //---------------------------------------------------------------------------
  //--------------- LOAD USING OLD DESCRIPTOR FORMAT --------------------------
  // Reading descriptor from file
  std::ifstream input;
  input.open(path, std::ios_base::binary);
  int header[3];  // 0-liczba polaczen miedzy stanami, 1-liczba punktow dla
                  // jednej pozy
  input.read((char*)header, sizeof(header));
  // double threshold;
  // input.read((char*) &threshold, sizeof(double));

  // Mapa polaczen miedzy stanami
  int statesNumber = header[0] + 1;
  bool* map = new bool[statesNumber * statesNumber];
  input.read((char*)map, sizeof(bool) * statesNumber * statesNumber);

  // tablica z punktami (x,y) dla deskryptorow. <serialized>
  double* buffer = new double[header[0] * header[1] * 2];
  input.read((char*)buffer, sizeof(double) * header[0] * header[1] * 2);
  input.close();

  //---------------------------------------------------------------------------
  //--------------- SAVE USING NEW DESCRIPTOR FORMAT --------------------------
  std::ofstream descriptor_file;
  descriptor_file.open(path, std::ios_base::binary);
  bool temp = descriptor_file.is_open();
  int size1 = header[0];  // Liczba przejœæ wa¿onych wektorem wejœciowym
  int size2 = header[1];  // Liczba punktow deskryptora dla kazdego stanu
  int descrType = header[2];

  descriptor_file.write((char*)&size1, 4);
  descriptor_file.write((char*)&size2, 4);
  descriptor_file.write((char*)&descrType, 4);
  descriptor_file.write((char*)&thresholdNew, 8);

  // Zapisz mape polaczen
  descriptor_file.write((char*)map, sizeof(bool) * statesNumber * statesNumber);

  // Wpisz spowrotem deskryptor
  descriptor_file.write((char*)buffer,
                        sizeof(double) * header[0] * header[1] * 2);

  descriptor_file.close();
  delete[] map;
  delete[] buffer;
}

void BehaviorFilter::SetFirstState(BehaviorState* first) {
  firstState = first;
  currentState = first;
}

BehaviorState* BehaviorFilter::GetFirstState() { return firstState; }

void BehaviorFilter::Save(char* path) {
  std::ofstream descriptor_file;
  descriptor_file.open(path, std::ios_base::binary);
  if (!descriptor_file.is_open())
    throw (std::string) "Couldn't open desired location. Probable access denied, check user privileges (may not work on ../ProgramFiles on NON Administrator account).";

  BehaviorState* iter = temp_StateHandles[(int)temp_StateHandles.size() - 1];
  int size1 = iter->sttNumber;  // Liczba stanów g³ównych -> pierwszy rozmiar
                                // acceptable input
  // int size2 = (int)iter->acceptableInput.size();//Liczba punktow deskryptora
  // dla kazdego stanu

  int sttsNumber = size1 + 1;

  descriptor_file.write((char*)&size1, 4);
  // descriptor_file.write((char*)&size2,4);
  descriptor_file.write((char*)&behaviorType, 4);

  //----- Zapisanie wektora z dlugosciami deskryptorow poz ----------
  int* sizes = new int[size1];
  for (int i = 1; i < sttsNumber; i++)
    sizes[i - 1] = (int)temp_StateHandles[i]->acceptableInput.size();
  descriptor_file.write((char*)sizes, size1 * sizeof(int));
  //-----Zapisanie wektora z progami wejsciowymi---------------------
  double* thresholds = new double[size1];
  for (int i = 0; i < size1; i++)
    thresholds[i] = temp_StateHandles[i + 1]->threshold;

  descriptor_file.write((char*)thresholds, sizeof(double) * size1);
  //-----------------------------------------------------------------
  //----Zapisanie mapy polaczen miedzy stanami-----------------------
  bool* map = new bool[sttsNumber * sttsNumber];
  for (int i = 0; i < sttsNumber * sttsNumber; i++) map[i] = false;

  this->firstState->MarkConnections(map, sttsNumber);

  descriptor_file.write((char*)map, sizeof(bool) * sttsNumber * sttsNumber);
  //-----------------------------------------------------------------
  //----Zapisanie wektora z oznaczeniami stanow wyjsciowych----------
  bool* islast = new bool[sttsNumber];
  for (int i = 0; i < sttsNumber; i++)
    islast[i] = temp_StateHandles[i]->lastState;

  descriptor_file.write((char*)islast, sizeof(bool) * sttsNumber);
  //-----------------------------------------------------------------
  //----Wpisz punkty deskryptora-------------------------------------
  this->firstState->SaveDescriptor(descriptor_file);

  descriptor_file.close();
  delete[] thresholds;
  delete[] islast;
  delete[] map;
}

void BehaviorFilter::SetThreshold(double thresh) {
  for (size_t i = 0; i < this->temp_StateHandles.size(); i++) {
    temp_StateHandles[i]->threshold = thresh;
  }
}

void BehaviorFilter::SetBehaviorType(int type, std::string message) {
  this->behaviorType = type;
  this->behaviorDescription = message;
  for (size_t i = 0; i < this->temp_StateHandles.size(); i++) {
    temp_StateHandles[i]->behType = type;
  }
}

void BehaviorFilter::Append(BehaviorFilter* appFilter) {
  // Dodaj po³¹czenia do struktury
  for (int i = 0; i < (int)appFilter->firstState->nextStates.size(); i++) {
    firstState->nextStates.push_back(appFilter->firstState->nextStates[i]);
  }
  // WskaŸniki prze³ó¿ do lokalnego wektora ze stanami
  for (int i = 1; i < (int)appFilter->temp_StateHandles.size(); i++) {
    temp_StateHandles.push_back(appFilter->temp_StateHandles[i]);
  }
  // Aktualizuj numery stanow
  for (int i = 0; i < (int)temp_StateHandles.size(); i++) {
    temp_StateHandles[i]->sttNumber = i;
  }
}
}  // namespace bd
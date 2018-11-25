#include "..\includes\detected_object.h"
// Project includes
#include "..\includes\BehaviorDescription.h"
#include "..\includes\logger.h"

namespace bd {

detected_object::detected_object(void) {
  human = false;
  border = false;
  prediction_state = false;
  prediction_life_time = 0;
  KFilter = new Kalman_Filter();
  history_counter = 0;
  outside_counter = 0;
  inside_counter = 0;
  is_inside_restricted_area = false;
  is_close_to = false;
  LoadBehaviorFilters();
}

detected_object::detected_object(detected_object *obj) {
  number = -1;
  human = false;
  prediction_state = false;
  prediction_life_time = 0;
  KFilter = new Kalman_Filter();
  this->KFilter->KF = obj->KFilter->KF;
  is_close_to = false;
  this->bFilter = obj->bFilter;
  this->eventSaved = obj->eventSaved;
  /*
  pos_x.resize(50);
  pos_z.resize(50);*/

  //	time = 0;
  last_x = 0;
  last_z = 0;
  speed = 0;
}

detected_object::~detected_object(void) {
  delete KFilter;
  for (size_t i = 0; i < marged_objects.size(); i++) {
    delete marged_objects[i];
    marged_objects.erase(marged_objects.begin() + i);
  }

  for (size_t i = 0; i < bFilter.size(); i++) {
    delete bFilter[i];
  }
  bFilter.clear();
}

void detected_object::calc_speed(double x, double z, int timex) {
  int history_time = 10;

  double distance;

  if (history_counter < history_time) { 
    pos_x.push_back(x);
    pos_z.push_back(z);
    time.push_back(timex);
    double x_accu = 0;
    double z_accu = 0;
    for (int i = 1; i < history_counter; i++) {
      x_accu += pos_x[i] - pos_x[i - 1];
      z_accu += pos_z[i] - pos_z[i - 1];
    }

    distance = sqrt(pow((x_accu), 2) + pow((z_accu), 2));
    history_counter++;
    speed = 0;
    if (history_counter > 1) speed = distance / ((timex - time[0]) * 0.04);
  } else {
    pos_x.erase(pos_x.begin());
    pos_z.erase(pos_z.begin());
    time.erase(time.begin());
    pos_x.push_back(x);
    pos_z.push_back(z);
    time.push_back(timex);

    double x_accu = 0;
    double z_accu = 0;

    for (int i = 1; i < history_time; i++) {
      x_accu += pos_x[i] - pos_x[i - 1];
      z_accu += pos_z[i] - pos_z[i - 1];
    }
    distance = sqrt(pow((x_accu), 2) + pow((z_accu), 2));
    speed = (distance / ((timex - time[0]) * 0.04) + speed) / 2;
  }
}

void detected_object::CheckBehavior() {
  std::string tmp_message;
  std::vector<bool> foundBehaviors;
  BehaviorDescription beh_descr;
  foundBehaviors.resize(beh_descr.GetBehaviorTypesCount());
  for (size_t i = 0; i < bFilter.size(); i++) {
    bFilter[i]->Check(behDescr, is_moving);
    if (bFilter[i]->found) foundBehaviors[bFilter[i]->behaviorType] = true;
  }
  for (int i = 0; i < (int)foundBehaviors.size(); i++) {
    if (foundBehaviors[i]) 
      tmp_message.append(beh_descr.FindBehavior(i).name + " ");
  }
  message = tmp_message;
}

void detected_object::SetObjNumber() {
  for (size_t i = 0; i < bFilter.size(); i++)
    bFilter[i]->objNumber = this->number;
}

std::vector<int> detected_object::DetectedBehaviors() {
  std::vector<int> result;
  for (size_t i = 0; i < bFilter.size(); i++) {
    if (bFilter[i]->found) {
      result.push_back(bFilter[i]->behaviorType);
    }
  }
  return result;
}

std::vector<bool> detected_object::IsFound() {
  std::vector<bool> answer;
  answer.resize(bFilter.size());
  for (size_t i = 0; i < bFilter.size(); i++)
    answer[i] = this->bFilter[i]->found;
  return answer;
}

void detected_object::LoadBehaviorFilters(bool enable) {
  auto descriptors = LoadDescriptorsList();
  for (const auto &descr : descriptors) {
    bFilter.push_back(new BehaviorFilter(descr));
    double temp_threshold;
    if (enable && load_data("parameters.txt", descr, temp_threshold))
      bFilter.back()->SetThreshold(temp_threshold);
  }

  eventSaved.resize(bFilter.size());
}

std::vector<std::string> detected_object::LoadDescriptorsList() {
  static bool first_load = true;
  static std::vector<std::string> descr_list;
  if (first_load) {
    std::ifstream descr_list_file("descriptors_list.txt");
    std::string temp;
    if (!descr_list_file.is_open()) 
      throw std::runtime_error("Missing 'descriptors_list.txt'.");
    while (!descr_list_file.eof()) {
      descr_list_file >> temp;
      descr_list.push_back(temp);
      std::string msg = "loading descriptor " + temp;
      LOG("detected_object", msg, LogLevel::kSetup);
    }
    first_load = false;
    descr_list_file.close();
  }
  return descr_list;
}

bool detected_object::detect_movement() {
  size_t movement_history = 25;
  if (KFilter->kalmanv.size() > movement_history) {
    for (size_t i = KFilter->kalmanv.size() - movement_history;
         i < KFilter->kalmanv.size(); i++)
      if (!isinside2(i)) return true;
    return false;
  } else {
    for (size_t i = 0; i < KFilter->kalmanv.size(); i++)
      if (!isinside2(i)) return true;
    return false;
  }
  return false;
}

inline bool detected_object::isinside(int i) {
  if ((KFilter->kalmanv[i].x < rect.x + rect.width) &&
      (KFilter->kalmanv[i].x > rect.x) && (KFilter->kalmanv[i].y > rect.y) &&
      (KFilter->kalmanv[i].y < rect.y + rect.height)) {
    return true;
  } else
    return false;
}

inline bool detected_object::isinside2(int i) {
  cv::Rect temp_rect;

  temp_rect.x = rect.x;
  temp_rect.y = rect.y + rect.height / 2;
  temp_rect.width = rect.width;
  temp_rect.height = rect.height;

  return KFilter->kalmanv[i].inside(temp_rect);
}

void detected_object::estimate_direction() {
  int vector_history = 50;
  int mean_members = 10;
  if (static_cast<int>(KFilter->kalmanv.size()) > vector_history) {
    double x1 = 0;
    double x2 = 0;
    double y1 = 0;
    double y2 = 0;
    for (size_t i = KFilter->kalmanv.size() - vector_history;
         i < KFilter->kalmanv.size() - vector_history + mean_members; i++) {
      x1 += KFilter->kalmanv[i].x;
      y1 += KFilter->kalmanv[i].y;
    }
    for (size_t i = KFilter->kalmanv.size() - mean_members;
         i < KFilter->kalmanv.size(); i++) {
      x2 += KFilter->kalmanv[i].x;
      y2 += KFilter->kalmanv[i].y;
    }
    x1 = x1 / mean_members;
    x2 = x2 / mean_members;
    y1 = y1 / mean_members;
    y2 = y2 / mean_members;

    x_movement = (x2 - x1) / (vector_history - mean_members);
    y_movement = (y2 - y1) / (vector_history - mean_members);
  } else {
    if (KFilter->kalmanv.size() > 2) {
      x_movement = (KFilter->kalmanv[KFilter->kalmanv.size() - 1].x -
                    KFilter->kalmanv[0].x) /
                   KFilter->kalmanv.size();
      y_movement = (KFilter->kalmanv[KFilter->kalmanv.size() - 1].y -
                    KFilter->kalmanv[0].y) /
                   KFilter->kalmanv.size();
    }
  }
}

void detected_object::ShowBehaviorStates(cv::Mat frame) {
  char temp_sttNum[200] = {""};
  std::ostrstream zapis(temp_sttNum, (int)sizeof(temp_sttNum), std::ios::app);
  for (size_t i = 0; i < bFilter.size(); i++) {
    zapis << " | " << bFilter[i]->GetCurrentStateNumber();
  }
  zapis << std::ends;
  putText(frame, temp_sttNum, cv::Point(this->rect.x + 1, this->rect.y + 10), 0,
          0.3, cv::Scalar(255, 255, 0));
}
}  // namespace bd
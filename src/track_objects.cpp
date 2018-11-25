#include "..\includes\track_objects.h"
#include "..\includes\DiffImage.h"

namespace bd {
using namespace std;
using namespace cv;

inline double GetDist(Point2f p1, Point2f p2) {
  return sqrt(((p1.x - p2.x) * (p1.x - p2.x)) +
              ((p1.y - p2.y) * (p1.y - p2.y)));
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
    if (objects_mask[i] == 0) {  // niepowizane
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
      } else {  // zostaly na srodku - sklejenie/zgubienie
        if (detected_objects[i]->prediction_life_time >
            max_prediction_length) {  // zbyt dluga predykcja
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
        temp_obj->next_pos = temp_obj->KFilter->statePt;
        global_counter_++;
        temp_obj->number = -1;  // global_counter;
        temp_obj->behDescr = behDescr[i];
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
    // wspó³rzêdne zewnêtrznych wierzcho³ków markera z pliku!!!
    vector<Point2f> obj_corners(4);  
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
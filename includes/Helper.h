#pragma once

namespace bd {

class Helper {
 private:
  double dx;    //"Szerokosc piksela"[m]
  double dy;    //"Wysokosc piksela"[m]
  double f;     // Ogniskowa soczewki w kamerze [m]
  double H;     // Wysokoœæ na której znajduje siê kamera [m]
  double alfa;  // Koniecznie w radianach [rad]
  int cam_w;    // Liczba punktów obrazu w wierszu
  int cam_h;    // Liczba punktów obrazu w kolumnie

 public:
  // Zmienne w których umieszczony zostanie po³o¿enie na p³aszczyŸnie po której
  // porusza siê obiekt.
  double X;
  double Z;
  double obj_h;

  Helper(void);

  /*Input:
  w - camera sensor width,
  h - camera sensor height,
  fo - camera focal length,
  Ha - camera latitude over ground level,
  alfaa - angle measured from horizont,
  cam_w - resolution width part, number of pixels in line,
  cam_h - resolution height part, number of pixels in column*/
  Helper(double w, double h, double fo, double Ha, double alfaa, int cam_w,
         int cam_h);

  /*Input:
  dx - physical horizontal pixel size [um],
  dy - physical vertical pixel size [um],
  fo - camera focal length,
  Ha - camera latitude over ground level,
  alfaa - angle measured from horizont,
  cam_w - resolution width part, number of pixels in line,
  cam_h - resolution height part, number of pixels in column*/
  Helper(double dx, double dy, double fo, double Ha, double alfaa, int cam_w,
         int cam_h, bool test);
  ~Helper(void);

  void CalculatePosition(int x, int y);
  void CalculateHeight(int x, int y);
  int GetImgWidth();
  int GetImgHeight();
  double Getdx();
  double Getdy();
  double GetAlpha();
  double GetFocal();
  double GetCamHeight();
  void Set(double dx, double dy, double fo, double Ha, double alfaa, int camW,
           int camH);
};
}  // namespace bd
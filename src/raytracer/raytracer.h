#pragma once
#include "../configfile/scenedata.h"

class Raytracer {
private:
  Vect origin;
  RenderingInfo *info;

  bool hits(Vect &origin, Vect &direction, Sphere *sphere, float min_t, float max_t, float &return_t);
  bool hits(Vect &origin, Vect &direction, Triangle *triangle, float min_t, float max_t, float &return_t);
  bool inShadow(Vect &origin, Vect &direction, float t_max);
  Color rayCast(Vect &origin, Vect &direction, int bounces);
public:
  Raytracer(Vect origin, RenderingInfo *info) : origin(origin), info(info) {}
  void render(Frame &frame);
  void setOrigin(Vect origin);
};
#pragma once
#include "../configfile/scenedata.h"

class Raytracer {
private:
  RenderingInfo *info;
  Vect origin;
public:
  Raytracer(Vect origin, RenderingInfo *info) : origin(origin), info(info) {}
  bool hits(Vect &direction, Sphere *sphere, float min_t, float max_t, float &return_t);
  bool hits(Vect &direction, Triangle *triangle, float min_t, float max_t, float &return_t);
  Color rayCast(Vect &direction);
};
#pragma once
#include "../configfile/scenedata.h"

class Raytracer {
private:
  RenderingInfo *info;
public:
  Raytracer(RenderingInfo *info) : info(info) {}
  bool hits(Vect &origin, Vect &direction, Sphere *sphere, float min_t, float max_t, float &return_t);
  bool hits(Vect &origin, Vect &direction, Triangle *triangle, float min_t, float max_t, float &return_t);
  Color rayCast(Vect &origin, Vect &direction);
};
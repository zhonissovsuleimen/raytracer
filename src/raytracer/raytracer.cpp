#include "raytracer.h"
#include "../matrix/matrix.h"

bool Raytracer::hits(Vect &origin, Vect &direction, Sphere *sphere, float min_t, float max_t, float &return_t) {
  Vect center = sphere->center;
  float radius = sphere->radius;

  float a = direction * direction;
  float b = (2 * direction) * (origin - center);
  float c = (origin - center) * (origin - center) - radius * radius;

  float discriminant = b * b - 4 * a * c;
  if (discriminant < 0.0f) { return false; }

  float sqrt_disc = sqrt(discriminant);
  float t1 = (-b + sqrt_disc) / (2 * a);
  float t2 = (-b - sqrt_disc) / (2 * a);
  float lesser_t = std::min(t1, t2);
  if (lesser_t > max_t || lesser_t < min_t) { return false; }

  return_t = lesser_t;
  return true;
}

bool Raytracer::hits(Vect &origin, Vect &direction, Triangle *triangle, float min_t, float max_t, float &return_t) {
  Vect p0 = triangle->p0;
  Vect p1 = triangle->p1;
  Vect p2 = triangle->p2;

  Matrix3D matrixA = {p0.x - p1.x, p0.x - p2.x, direction.x,
                      p0.y - p1.y, p0.y - p2.y, direction.y,
                      p0.z - p1.z, p0.z - p2.z, direction.z};

  float detA = matrixA.deteminant();

  Matrix3D matrixT = {p0.x - p1.x, p0.x - p2.x, p0.x - origin.x,
                      p0.y - p1.y, p0.y - p2.y, p0.y - origin.y,
                      p0.z - p1.z, p0.z - p2.z, p0.z - origin.z};

  float t = matrixT.deteminant() / detA;
  if (t < min_t || t >= max_t) { return false; }

  Matrix3D matrixGamma = {p0.x - p1.x, p0.x - origin.x, direction.x,
                          p0.y - p1.y, p0.y - origin.y, direction.y,
                          p0.z - p1.z, p0.z - origin.z, direction.z};

  float gamma = matrixGamma.deteminant() / detA;
  if (gamma < 0.0f || gamma > 1.0f) { return false; }

  Matrix3D matrixBeta = {p0.x - origin.x, p0.x - p2.x, direction.x,
                         p0.y - origin.y, p0.y - p2.y, direction.y,
                         p0.z - origin.z, p0.z - p2.z, direction.z};

  float beta = matrixBeta.deteminant() / detA;
  if (beta < 0.0f || beta > 1.0f - gamma) { return false; }
  
  return_t = t;
  return true;
}

Color Raytracer::rayCast(Vect &origin, Vect &direction) {
  float t = std::numeric_limits<float>::max();
  Material material;
  Vect n;

  std::vector<Sphere *> spheres = info->spheres;
  for (int i = 0; i < spheres.size(); i++) {
    float return_t;
    if (!hits(origin, direction, spheres[i], 0.0f, std::numeric_limits<float>::max(), return_t) 
        || return_t >= t) { continue; }
    t = return_t;
    material = *spheres[i]->material;

    Vect hit = origin + direction * t;
    n = (hit - spheres[i]->center).normalize();
  }

  std::vector<Triangle *> triangles = info->triangles;
  for (int i = 0; i < triangles.size(); i++) {
    float return_t;
    if (! hits(origin, direction, triangles[i], 0.0f, std::numeric_limits<float>::max(), return_t) 
        || return_t >= t) { continue; }
    t = return_t;
    material = *triangles[i]->material;

    Vect a = triangles[i]->p0;
    Vect b = triangles[i]->p1;
    Vect c = triangles[i]->p2;

    Vect ab = {b.x - a.x, b.y - a.y, b.z - a.z};
    Vect ac = {c.x - a.x, c.y - a.y, c.z - a.z};
    float nx = ab.y * ac.z - ab.z * ac.y;
    float ny = ab.z * ac.x - ab.x * ac.z;
    float nz = ab.x * ac.y - ab.y * ac.x;
    n = Vect{-nx, -ny, -nz}.normalize();
  }
  
  if (t == std::numeric_limits<float>::max()) { return { 0.0f, 0.0f, 0.0f }; }


  Vect hit = origin + direction * t;
  Color color = {material.color.r, material.color.g, material.color.b};
  float coef_phong = 0.0f;

  std::vector<Light*> point_lights = info->point_lights;
  for (int i = 0; i < point_lights.size(); i++) {
    //checking for shadow
    bool inShadow = false;
    float t_min = 0.0001f;

    for(int i = 0; i < spheres.size(); i++){
      float return_t;
      if(hits(hit, point_lights[i]->position - hit, spheres[i], t_min, 1.0f, return_t)){
        inShadow = true;
        break;
      }
    }
    if(inShadow){ continue; }

    for(int i = 0; i < triangles.size(); i++){
      float return_t;
      if(hits(hit, point_lights[i]->position - hit, triangles[i], t_min, 1.0f, return_t)){
        inShadow = true;
        break;
      }
    }
    if(inShadow){ continue; }

    Vect l = (point_lights[i]->position - hit).normalize();
    Vect v = (origin - hit).normalize();
    Vect h = (v + l).normalize();

    float coef_diffuse = std::max(0.0f, n * l);
    float coef_specular = material.glossiness * std::max(0.0f, std::pow(n * h, material.p));
    coef_phong += point_lights[i]->intensity/100 * (coef_diffuse + coef_specular);
  }
  color = color * (info->ambient + coef_phong);

  return color;
}
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

  float detA = matrixA.det();

  Matrix3D matrixT = {p0.x - p1.x, p0.x - p2.x, p0.x - origin.x,
                      p0.y - p1.y, p0.y - p2.y, p0.y - origin.y,
                      p0.z - p1.z, p0.z - p2.z, p0.z - origin.z};

  float t = matrixT.det() / detA;
  if (t < min_t || t >= max_t) { return false; }

  Matrix3D matrixGamma = {p0.x - p1.x, p0.x - origin.x, direction.x,
                          p0.y - p1.y, p0.y - origin.y, direction.y,
                          p0.z - p1.z, p0.z - origin.z, direction.z};

  float gamma = matrixGamma.det() / detA;
  if (gamma < 0.0f || gamma > 1.0f) { return false; }

  Matrix3D matrixBeta = {p0.x - origin.x, p0.x - p2.x, direction.x,
                         p0.y - origin.y, p0.y - p2.y, direction.y,
                         p0.z - origin.z, p0.z - p2.z, direction.z};

  float beta = matrixBeta.det() / detA;
  if (beta < 0.0f || beta > 1.0f - gamma) { return false; }
  
  return_t = t;
  return true;
}

bool Raytracer::inShadow(Vect &origin, Vect &direction, float t_max) {
  float t_min = 0.0001f;

  std::vector<Sphere *> spheres = info->spheres;
  for(int i = 0; i < spheres.size(); i++){
    float return_t;
    if(hits(origin, direction, spheres[i], t_min, t_max, return_t)){ return true; }
  }

  std::vector<Triangle *> triangles = info->triangles;
  for(int i = 0; i < triangles.size(); i++){
    float return_t;
    if(hits(origin, direction, triangles[i], t_min, t_max, return_t)){ return true; }
  }

  return false;
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
  
  Color color = material.color;
  float diffuse = 0.0f;
  float specular = 0.0f;

  Vect hit = origin + direction * t;
  Vect v = (origin - hit).normalize();
  bool hasDirLight = info->dir_light != nullptr;

  std::vector<Light*> point_lights = info->point_lights;
  for (int i = 0; i <= point_lights.size(); i++) {
    bool isDirLight = i == point_lights.size();
    if(isDirLight && !hasDirLight){ break; }

    Vect direction = (hasDirLight && isDirLight) ? -info->dir_light->direction : (point_lights[i]->position - hit);
    float intensity = (hasDirLight && isDirLight) ? info->dir_light->h_intensity : point_lights[i]->intensity;
    float t_max = (hasDirLight && isDirLight) ? std::numeric_limits<float>::max() : 1.0f;

    if(inShadow(hit, direction, t_max)){ continue; }

    Vect l = direction.normalize();
    Vect h = (v + l).normalize();

    float distanceSquared = direction.mag() * direction.mag();
    float irradiance = (hasDirLight && isDirLight) ? intensity * n*l : intensity / distanceSquared;

    diffuse += irradiance * std::max(0.0f, n * l);
    specular += irradiance * material.glossiness * pow(std::max(0.0f, n * h), material.p);
  }

  color = color * (info->ambient + diffuse) + Color{1.0f, 1.0f, 1.0f} * specular;
  return color;
}

void Raytracer::render(Frame &frame) {
  //viewplane borders
  float min = -1.0f;
  float max = 1.0f;
  float aspect_ratio = (float)WIDTH / HEIGHT;

  for(int y = 0; y < HEIGHT; y++){
    for(int x = 0; x < WIDTH; x++){
      float pixel_x = min + (max - min) * ((float)x / WIDTH);
      float pixel_y = min + (max - min) * ((float)y / HEIGHT);
      pixel_y /= aspect_ratio;
      
      Vect d = Vect{pixel_x, pixel_y, info->focal_length };
      Color c = rayCast(origin, d);
      
      frame.setColor(x, y, c);
    }
  }
}

void Raytracer::setOrigin(Vect origin) { this->origin = origin; }
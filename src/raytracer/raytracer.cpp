#include "raytracer.h"

bool Raytracer::hits(Vect &origin, Vect &direction, Sphere *sphere, float min_t, float max_t, float &return_t) {
  Vect center = sphere->center;
  float radius = sphere->radius;

  Vect oc = origin - center;

  float a = direction * direction;
  float b = (2 * direction) * oc;
  float c = oc * oc - radius * radius;

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

bool Raytracer::hits(Vect &origin, Vect &direction, Triangle *triangle, float t_min, float t_max, float &return_t) {
  Vect p0 = triangle->p0;
  Vect p1 = triangle->p1;
  Vect p2 = triangle->p2;

  float a = p0.x - p1.x;
  float b = p0.y - p1.y;
  float c = p0.z - p1.z;
  float d = p0.x - p2.x;
  float e = p0.y - p2.y;
  float f = p0.z - p2.z;
  float j = p0.x - origin.x;
  float k = p0.y - origin.y;
  float l = p0.z - origin.z;

  float ei_hf = e * direction.z - direction.y * f;
  float gf_di = direction.x * f - d * direction.z;
  float dh_eg = d * direction.y - e * direction.x;
  float ak_jb = a * k - j * b;
  float jc_al = j * c - a * l;
  float bl_kc = b * l - k * c;

  float M = a * ei_hf + b * gf_di + c * dh_eg;
  float t = -(f * ak_jb + e * jc_al + d * bl_kc) / M;
  if (t < t_min || t >= t_max) { return false; }

  float gamma = (direction.z * ak_jb + direction.y * jc_al + direction.x * bl_kc) / M;
  if (gamma < 0.0f || gamma > 1.0f) { return false; }

  float beta = (j * ei_hf + k * gf_di + l * dh_eg) / M;
  if (beta < 0.0f || beta > 1.0f - gamma) { return false; }
  
  return_t = t;
  return true;
}

bool Raytracer::inShadow(Vect &origin, Vect &direction, float t_max) {
  float t_min = 0.0001f;

  float return_t;
  std::vector<Sphere *> spheres = info->spheres;
  for(int i = 0; i < spheres.size(); i++){
    if(hits(origin, direction, spheres[i], t_min, t_max, return_t)){ return true; }
  }

  std::vector<Triangle *> triangles = info->triangles;
  for(int i = 0; i < triangles.size(); i++){
    if(hits(origin, direction, triangles[i], t_min, t_max, return_t)){ return true; }
  }

  return false;
}

Color Raytracer::rayCast(Vect &origin, Vect &direction, int bounces) {
  float t = std::numeric_limits<float>::max();
  Material material;
  Vect n;

  std::vector<Sphere *> spheres = info->spheres;
  for (int i = 0; i < spheres.size(); i++) {
    float return_t;
    if (!hits(origin, direction, spheres[i], 0.0f, std::numeric_limits<float>::max(), return_t) || return_t >= t) { continue; }
    t = return_t;
    material = *spheres[i]->material;

    Vect hit = origin + direction * t;
    n = (hit - spheres[i]->center).normalize();
  }

  std::vector<Triangle *> triangles = info->triangles;
  for (int i = 0; i < triangles.size(); i++) {
    float return_t;
    if (! hits(origin, direction, triangles[i], 0.0f, std::numeric_limits<float>::max(), return_t) || return_t >= t) { continue; }
    t = return_t;
    material = *triangles[i]->material;

    Vect a = triangles[i]->p0;
    Vect b = triangles[i]->p1;
    Vect c = triangles[i]->p2;

    Vect ab = b - a;
    Vect ac = c - a;
    float nx = ab.y * ac.z - ab.z * ac.y;
    float ny = ab.z * ac.x - ab.x * ac.z;
    float nz = ab.x * ac.y - ab.y * ac.x;
    n = Vect{nx, ny, nz}.normalize();
    if(n * direction > 0.0f) { n = n * -1.0f; }
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

    if(info->shadows && inShadow(hit, direction, t_max)){ continue; }

    Vect l = direction.normalize();
    Vect h = (v + l).normalize();

    float distanceSquared = direction.mag() * direction.mag();
    float irradiance = (hasDirLight && isDirLight) ? (intensity * n*l) : (intensity / distanceSquared);

    diffuse += irradiance * std::max(0.0f, n * l);
    specular += irradiance * material.glossiness * pow(std::max(0.0f, n * h), material.p);
  }

  Color reflection;
  if(bounces > 0 && material.mirror > 0.0f){
    Vect r = direction - n * 2.0f * (direction * n);
    reflection = rayCast(hit, r, bounces-1) * material.mirror;
  }

  color = color * (info->ambient + diffuse);
  color += Color{1.0f, 1.0f, 1.0f} * specular;
  color += reflection;

  return color;
}

void Raytracer::render(Frame &frame) {
  //viewplane borders
  float min = -1.0f;
  float max = 1.0f;

  int bounces = 3;
  float aspect_ratio = (float)WIDTH / HEIGHT;

  float rad = theta * 3.1415926f / 180.0f;
  float sin_theta = sin(rad);
  float cos_theta = cos(rad);

  for(int y = 0; y < HEIGHT; y++){
    for(int x = 0; x < WIDTH; x++){
      float pixel_x = min + (max - min) * ((float)x / WIDTH);
      float pixel_y = min + (max - min) * ((float)y / HEIGHT);
      pixel_y /= aspect_ratio;
      
      float z = info->focal_length;
      pixel_x = cos_theta * pixel_x - sin_theta * z;
      z = sin_theta * pixel_x + cos_theta * z;

      Vect d = Vect{pixel_x, pixel_y, z };
      Color c = rayCast(origin, d, bounces);

      frame.setColor(x, y, c);
    }
  }
}

//void Raytracer::setOrigin(Vect origin) { this->origin = origin; }

void Raytracer::setTheta(float theta) { this->theta = theta; }
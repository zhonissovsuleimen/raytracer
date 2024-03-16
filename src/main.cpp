#include <iostream>
#include <limits>
#include <string>

#include "configfile/parser.h"
#include "configfile/scenedata.h"
#include "matrix/matrix.h"
#include "renderer/renderer.h"
#include "renderer/renderer_types.h"

void drawSphere(Frame &frame, Sphere *sphere, Vect &origin, RenderingInfo *info, std::vector<std::vector<float>> &zbuffer) {
  Vect center = sphere->center;
  float radius = sphere->radius;

  for (int y = 0; y < HEIGHT; y++) {
    for (int x = 0; x < WIDTH; x++) {
      Vect pixel = {x + 0.5f, y + 0.5f, origin.z + (WIDTH * info->focal_length)};
      Vect d = pixel - origin;

      float a = d * d;
      float b = (2 * d) * (origin - center);
      float c = (origin - center) * (origin - center) - radius * radius;

      float discriminant = b * b - 4 * a * c;
      if (discriminant < 0) { continue; }

      float sqrt_disc = sqrt(discriminant);
      float t1 = (-b + sqrt_disc) / (2 * a);
      float t2 = (-b - sqrt_disc) / (2 * a);
      float lesser_t = (t1 < t2) ? t1 : t2;

      if (lesser_t > zbuffer[x][y]) { continue; }

      zbuffer[x][y] = lesser_t;
      frame.setColor(x, y, sphere->material->color);
    }
  }
}

void drawTriangle(Frame &frame, Triangle *triangle, Vect &origin, RenderingInfo *info, std::vector<std::vector<float>> &zbuffer) {
  Vect p0 = triangle->p0;
  Vect p1 = triangle->p1;
  Vect p2 = triangle->p2;

  for (int y = 0; y < HEIGHT; y++) {
    for (int x = 0; x < WIDTH; x++) {
      Vect pixel = {x + 0.5f, y + 0.5f, origin.z + (WIDTH * info->focal_length)};
      Vect d = pixel - origin;

      Matrix3D matrixA = {p0.x - p1.x, p0.x - p2.x, d.x,
                          p0.y - p1.y, p0.y - p2.y, d.y,
                          p0.z - p1.z, p0.z - p2.z, d.z};

      float detA = matrixA.deteminant();

      Matrix3D matrixT = {p0.x - p1.x, p0.x - p2.x, p0.x - origin.x,
                          p0.y - p1.y, p0.y - p2.y, p0.y - origin.y,
                          p0.z - p1.z, p0.z - p2.z, p0.z - origin.z};

      float t = matrixT.deteminant() / detA;
      if (t < 0.0f || t > zbuffer[x][y]) { continue; }

      Matrix3D matrixGamma = {p0.x - p1.x, p0.x - origin.x, d.x,
                              p0.y - p1.y, p0.y - origin.y, d.y,
                              p0.z - p1.z, p0.z - origin.z, d.z};

      float gamma = matrixGamma.deteminant() / detA;
      if (gamma < 0.0f || gamma > 1.0f) { continue; }

      Matrix3D matrixBeta = {p0.x - origin.x, p0.x - p2.x, d.x,
                             p0.y - origin.y, p0.y - p2.y, d.y,
                             p0.z - origin.z, p0.z - p2.z, d.z};

      float beta = matrixBeta.deteminant() / detA;
      if (beta < 0.0f || beta > 1.0f - gamma) { continue; }

      zbuffer[x][y] = t;
      frame.setColor(x, y, triangle->material->color);
    }
  }
}

int main(int argc, char **argv) {
  if (argc < 2) {
    std::cout << "Usage: " << argv[0] << " <config file>" << std::endl;
    return 1;
  }

  Parser parser;
  RenderingInfo *info = parser.parseFile(argv[1]);
  if (info == nullptr) {
    std::cout << "Failed to parse file" << std::endl;
    return 1;
  }

  Frame frame;
  Renderer renderer;

  // viewplane is from 0 to WIDTH and 0 to HEIGHT
  // camera/origin is at the center of the viewplane by x,y and z = -800.0f
  Vect origin = {WIDTH / 2, HEIGHT / 2, -800.0f};
  std::vector<std::vector<float>> zbuffer(WIDTH, std::vector<float>(HEIGHT, std::numeric_limits<float>::max()));

  std::vector<Sphere *> spheres = info->spheres;
  for (int i = 0; i < spheres.size(); i++) {
    drawSphere(frame, spheres[i], origin, info, zbuffer);
  }

  std::vector<Triangle *> triangles = info->triangles;
  for (int i = 0; i < triangles.size(); i++) {
    drawTriangle(frame, triangles[i], origin, info, zbuffer);
  }

  if (!renderer.init(frame)) {
    std::cout << "Failed to init renderer" << std::endl;
    return 1;
  }
  while (renderer.render()) {
    // frame.clear();
    // renderer.changeFrame(frame);
  }
}
#include <iostream>
#include <string>
#include <limits>

#include "configfile/parser.h"
#include "configfile/scenedata.h"
#include "matrix/matrix.h"
#include "renderer/renderer.h"
#include "renderer/renderer_types.h"

class TestSquare {
private:
  float n;

public:
  TestSquare() : n(0) {}
  void drawSquare(Frame &frame);
};

void TestSquare::drawSquare(Frame &frame) {
  n += 0.1;
  float sn = sin(n);
  float cn = cos(n);
  int x = 400 + (int)(sn * 100);
  int y = 300 + (int)(cn * 100);
  Color c = {1.0, 0, 0};
  for (int i = 0; i < 10; i++) {
    for (int j = 0; j < 10; j++) {
      frame.setColor(x + i, y + j, c);
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

  //viewplane is from 0 to WIDTH and 0 to HEIGHT
  //camera/origin is at the center of the viewplane by x,y and z = -800.0f
  Vect origin = {WIDTH/2, HEIGHT/2, -800.0f};
  Color color = {0.0f, 0.0f, 0.0f};

  for (int y = 0; y < HEIGHT; y++) {
    for (int x = 0; x < WIDTH; x++) {
      float min_t = std::numeric_limits<float>::max();
      float local_t = std::numeric_limits<float>::max(); 
      
      Vect pixel = {x + 0.5f, y + 0.5f, origin.z + (WIDTH*info->focal_length)};
      Vect d = pixel - origin;

      std::vector<Sphere *> spheres = info->spheres;
      for (int i = 0; i < spheres.size(); i++) {
        float a = d * d;
        float b = (2 * d) * (origin - spheres[i]->center);
        float c =
            (origin - spheres[i]->center) * (origin - spheres[i]->center) -
            spheres[i]->radius * spheres[i]->radius;

        float discriminant = b * b - 4 * a * c;
        if (discriminant >= 0) {
          float sqrt_disc = sqrt(discriminant);
          float t1 = (-b + sqrt_disc) / (2 * a);
          float t2 = (-b - sqrt_disc) / (2 * a);

          local_t = (t1 < t2) ? t1 : t2;
          if(local_t < min_t) {
            min_t = local_t;
            color = spheres[i]->material->color;
          }
        }
      }

      std::vector<Triangle *> triangles = info->triangles;
      for (int i = 0; i < triangles.size(); i++) {
        Vect p0 = triangles[i]->p0;
        Vect p1 = triangles[i]->p1;
        Vect p2 = triangles[i]->p2;

        Matrix3D matrixA = {
          p0.x - p1.x, p0.x - p2.x, d.x,
          p0.y - p1.y, p0.y - p2.y, d.y,
          p0.z - p1.z, p0.z - p2.z, d.z
        };

        float detA = matrixA.deteminant();

        Matrix3D matrixT = {
          p0.x - p1.x, p0.x - p2.x, p0.x - origin.x,
          p0.y - p1.y, p0.y - p2.y, p0.y - origin.y,
          p0.z - p1.z, p0.z - p2.z, p0.z - origin.z
        };

        float local_t = matrixT.deteminant() / detA;
        if (local_t < 0.0f || local_t > min_t) { continue; }

        Matrix3D matrixGamma = {
          p0.x - p1.x, p0.x - origin.x, d.x,
          p0.y - p1.y, p0.y - origin.y, d.y,
          p0.z - p1.z, p0.z - origin.z, d.z
        };

        float gamma = matrixGamma.deteminant() / detA;
        if (gamma < 0.0f || gamma > 1.0f) { continue; }

        Matrix3D matrixBeta = {
          p0.x - origin.x, p0.x - p2.x, d.x,
          p0.y - origin.y, p0.y - p2.y, d.y,
          p0.z - origin.z, p0.z - p2.z, d.z
        };

        float beta = matrixBeta.deteminant() / detA;
        if (beta < 0.0f || beta > 1.0f - gamma) { continue; }

        if (local_t < min_t) {
          min_t = local_t;
          color = triangles[i]->material->color;
        }
      }

      if(min_t != std::numeric_limits<float>::max()) {
        frame.setColor(x, y, color);
      }
    }
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
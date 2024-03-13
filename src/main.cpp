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

  Vect origin = {0.0f, 0.0f, (WIDTH / info->focal_length)};

  float xdiff = WIDTH / 2;
  float ydiff = HEIGHT / 2;

  for (int y = -ydiff; y < HEIGHT / 2; y++) {
    for (int x = -xdiff; x < WIDTH / 2; x++) {
      float tSphere = std::numeric_limits<float>::max();

      Vect pixel = {x + 0.5f, y + 0.5f, 0.0f};
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

          tSphere = std::min(t1, t2);
          frame.setColor(x + xdiff, y + ydiff, spheres[i]->material->color);
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
          p0.x - p1.x, p0.x - origin.x, d.x,
          p0.y - p1.y, p0.y - origin.y, d.y,
          p0.z - p1.z, p0.z - origin.z, d.z
        };

        float t = matrixT.deteminant() / detA;
        if (t < 0.0f || t > tSphere) { continue; }

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

        frame.setColor(x + xdiff, y + ydiff, triangles[i]->material->color);
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
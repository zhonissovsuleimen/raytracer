#include <iostream>
#include <string>

#include "configfile/parser.h"
#include "configfile/scenedata.h"
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

  Vect origin = {0.0f, 0.0f, -(WIDTH/info->focal_length)};

  for(int y = 0; y < HEIGHT; y++) {
    for(int x = 0; x < WIDTH; x++) {
      Vect pixel = {x + 0.5f, y + 0.5f, 0.0f};
      Vect d = pixel - origin;

      std::vector<Sphere*> spheres = info->spheres;
      for(int i = 0; i < spheres.size(); i++){
        float a = d*d;
        float b = (2 * d) * (origin - spheres[i]->center);
        float c = (origin - spheres[i]->center) * (origin - spheres[i]->center) - spheres[i]->radius * spheres[i]->radius;

        float discriminant = b*b - 4*a*c;
        if(discriminant >= 0){
          float sqrt_disc = sqrt(discriminant);
          float t1 = (-b + sqrt_disc) / (2*a);
          float t2 = (-b - sqrt_disc) / (2*a);
          
          float t = std::min(t1, t2);
          frame.setColor(x, y, spheres[i]->material->color);
        }
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
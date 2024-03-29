#include "configfile/parser.h"
#include "renderer/renderer.h"
#include "raytracer/raytracer.h"

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

  Raytracer raytracer({0.0f, 0.0f, 0.0f}, info);
  raytracer.render(frame);

  if (!renderer.init(frame)) {
    std::cout << "Failed to init renderer" << std::endl;
    return 1;
  }

  float theta = 0.0f;
  while (renderer.render()) {
    frame.clear();
    raytracer.setTheta(theta);
    raytracer.render(frame);
    renderer.changeFrame(frame);

    theta += 1.0f;
    if (theta > 15.0f){
      theta = -15.0f;
    }
  }
}
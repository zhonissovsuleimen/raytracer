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

  float x = -1.0f;
  Raytracer raytracer({x, 0.0f, 0.0f}, info);
  raytracer.render(frame);

  if (!renderer.init(frame)) {
    std::cout << "Failed to init renderer" << std::endl;
    return 1;
  }
  while (renderer.render()) {
    frame.clear();
    x += 0.33f;
    if (x > 1.0f) { x = -1.0f; } 

    raytracer.setOrigin({x, 0.0f, 0.0f});
    raytracer.render(frame);
    renderer.changeFrame(frame);
  }
}
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

  Vect origin = {0.0f, 0.0f, 0.0f};
  Raytracer raytracer(origin, info);
  float min = -1.0f;
  float max = 1.0f;

  for(int y = 0; y < HEIGHT; y++){
    for(int x = 0; x < WIDTH; x++){
      Vect pixel = {(min + 2*(x + 0.5f)/WIDTH), min + 2*(y + 0.5f)/HEIGHT, info->focal_length};
      Vect d = pixel - origin;
      Color c = raytracer.rayCast(d);
      
      frame.setColor(x, y, c);
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
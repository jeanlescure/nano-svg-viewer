#ifndef APPLICATION_H
#define APPLICATION_H

#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <cairo/cairo.h>
#include <cairo/cairo-svg.h>
#include <librsvg/rsvg.h>

#include <stdio.h>
#include <chrono>
#include <thread>
#include <string>
#include <vector>
#include <sstream>
#include <iostream>

#include <linmath.h>

class Application {
  public:
    Application();

    int open();
    void close();

    void init();
    void runLoop();
    void cleanUp();

    int Debug = 0;

  private:
    bool should_close = false;

    double fpsLimit;
    double lastFrameTime;
    int display_w, display_h;
    vec4 clear_color;
    double now;
    GLFWwindow *window = nullptr;
    RsvgDimensionData svg_dim;
    std::vector<guint8 *> frames;

    guint8 *load_frame(const char *fileName);
};

#endif //APPLICATION_H

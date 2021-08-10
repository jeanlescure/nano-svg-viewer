#include "Application.h"

Application *gApp;

Application::Application() {
  gApp = this;
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
    gApp->close();
  }
}

int Application::open() {
  if (!glfwInit()) {
    return 1;
  }

  glfwWindowHint(GLFW_DECORATED, 0);
  glfwWindowHint(GLFW_FLOATING, 1);
  glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, !Debug);

  window = glfwCreateWindow(500, 500, "GL WINDOW", NULL, NULL);
  if (window == NULL) {
    return 2;
  }

  glfwSetWindowPos(window, 50, 200);

  glfwSetKeyCallback(window, key_callback);

  return 0;
}

void Application::close() {
  glfwSetWindowShouldClose(window, 1);
  should_close = true;
}

guint8 *Application::load_frame(const char *fileName) {
  GError *error = NULL;
  RsvgHandle *handle = rsvg_handle_new_from_file(fileName, &error);

  if(error) {
    exit(3);
  }

  rsvg_handle_get_dimensions(handle, &svg_dim);

  glfwSetWindowSize(window, svg_dim.width, svg_dim.height);

  int stride = cairo_format_stride_for_width(CAIRO_FORMAT_ARGB32, svg_dim.width);
  guint8 *image = (guint8 *) g_malloc(stride * svg_dim.height);
  memset(image, 0, (size_t) stride * svg_dim.height);

  cairo_surface_t *target = cairo_image_surface_create_for_data(image,
                                                                CAIRO_FORMAT_ARGB32,
                                                                svg_dim.width, svg_dim.height,
                                                                stride);

  if (cairo_surface_status(target) !=	CAIRO_STATUS_SUCCESS) {
    exit(4);
  }

  cairo_t *context = cairo_create(target);

  if(rsvg_handle_render_cairo(handle, context) != 1) {
    exit(5);
  }

  cairo_destroy (context);
  cairo_surface_destroy (target);

  return image;
}

GLuint tId;

void Application::init() {
  clear_color[0] = 0.0;
  clear_color[1] = 0.0;
  clear_color[2] = 0.0;
  clear_color[3] = 0.0;

  fpsLimit = 1.0 / 120.0;
  lastFrameTime = 0;   // number of seconds since the last frame

  glfwMakeContextCurrent(window);
  gladLoadGL(glfwGetProcAddress);
  glfwSwapInterval(1); // Enable vsync

  // NOTE: OpenGL error checks have been omitted for brevity

  for (int i = 0; i < 120; i++) {
    std::stringstream ss;
    ss << "las12tile4/" << i << ".svg";

    frames.push_back(
      load_frame(ss.str().c_str())
    );
  }

  glGenTextures(1, &tId);
  glDisable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_TEXTURE_2D);
  glfwGetFramebufferSize(window, &display_w, &display_h);
  glViewport(0, 0, display_w, display_h);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0., display_w, display_h, 0., -1., 1.);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
}

void Application::runLoop() {
  int current_frame = 0;

  while(!should_close) {
    now = glfwGetTime();

    if ((now - lastFrameTime) < fpsLimit) {
        std::chrono::milliseconds dura((int)(fpsLimit * 100));
        std::this_thread::sleep_for(dura);
    } else {
      glfwPollEvents();

      glClearColor(clear_color[0], clear_color[1], clear_color[2], clear_color[3]);
      glClear(GL_COLOR_BUFFER_BIT);

      current_frame += 1;
      if (current_frame >= 120) {
        current_frame = 0;
      }

      glBindTexture(GL_TEXTURE_2D, tId);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0.);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0.);
      glTexImage2D(
        GL_TEXTURE_2D,
        0.,
        GL_RGBA8,
        svg_dim.width, svg_dim.height,
        0.,
        GL_BGRA,
        GL_UNSIGNED_BYTE,
        frames[current_frame]
      );

      glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 0.0f);
        glVertex2f(0.0f, 0.0f);
        glTexCoord2f(1.0f, 0.0f);
        glVertex2f((float) svg_dim.width, 0.0f);
        glTexCoord2f(1.0f, 1.0f);
        glVertex2f((float) svg_dim.width , (float) svg_dim.height);
        glTexCoord2f(0.0f, 1.0f);
        glVertex2f(0.0f, (float) svg_dim.height);
      glEnd();

      // draw your frame here
      glfwSwapBuffers(window);
      glFinish();

      // only set lastFrameTime when you actually draw something
      lastFrameTime = now;
    }
  }
}

void Application::cleanUp() {
  glfwDestroyWindow(window);
  glfwTerminate();
}

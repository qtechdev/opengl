#include <array>
#include <chrono>
#include <ctime>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "glad.h"
#include <GLFW/glfw3.h>

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "gl/rect.hpp"
#include "gl/shader_program.hpp"
#include "gl/texture.hpp"
#include "gl/window.hpp"
#include "util/error.hpp"
#include "util/file_io.hpp"
#include "util/xdg.hpp"

const int window_width = 640;
const int window_height = 480;

const int gl_major_version = 3;
const int gl_minor_version = 3;

#ifdef DEBUG
namespace xdg {
  std::optional<xdg::path> get_data_path(
    const xdg::base &b, const std::string &n, const std::string &p,
    fio::log_stream_f &log_stream, const bool create=false
  );
}
namespace fio {
  std::optional<xdg::path> read(
    const xdg::path &path, fio::log_stream_f &log_stream
  );
}
Texture loadTexture(const xdg::path &path, fio::log_stream_f &log_stream);
#endif

void processInput(GLFWwindow *window);
std::array<glm::mat4, 3> fullscreen_rect_matrices(const int w, const int h);

int main(int argc, const char *argv[]) {
  xdg::base base_dirs = xdg::get_base_directories();

  #ifdef DEBUG
  auto log_path = xdg::get_data_path(base_dirs, "qogl", "logs/qogl.log", true);
  fio::log_stream_f log_stream(*log_path);
  std::cout << "RUNNING IN DEBUG MODE\nLOGGING TO " << *log_path << std::endl;
  #endif

  GLFWwindow *window = createWindow(
    gl_major_version, gl_minor_version, true, window_width, window_height,
    "Hello, OpenGL!"
  );

  #ifdef DEBUG
  log_stream << "Attempting to create context: ";
  log_stream << gl_major_version << "." << gl_minor_version << "...\n";
  #endif

  if (window == nullptr) {
    #ifdef DEBUG
    log_stream << "failed to create window\n";
    #endif

    glfwDestroyWindow(window);
    return to_underlying(error_code_t::window_failed);
  }

  glfwMakeContextCurrent(window);

  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    #ifdef DEBUG
    log_stream << "failed to initialise GLAD\n";
    #endif

    return to_underlying(error_code_t::glad_failed);
  }

  #ifdef DEBUG
  log_stream << "OpenGL Version: " << glGetString(GL_VERSION) << "\n";
  log_stream << "GLFW Version: " << glfwGetVersionString() << "\n";
  #endif

  glViewport(0, 0, window_width, window_height);
  glClearColor(0.1, 0.1, 0.2, 1.0);

  auto v_shader_path = xdg::get_data_path(
    base_dirs, "qogl", "shaders/tex/vshader.glsl"
    #ifdef DEBUG
    , log_stream
    #endif
  );
  auto v_shader_string = fio::read(*v_shader_path);

  auto f_shader_path = xdg::get_data_path(
    base_dirs, "qogl", "shaders/tex/fshader.glsl"
    #ifdef DEBUG
    , log_stream
    #endif
  );
  auto f_shader_string = fio::read(*f_shader_path);

  GLuint v_shader = createShader(GL_VERTEX_SHADER, *v_shader_string);
  GLuint f_shader = createShader(GL_FRAGMENT_SHADER, *f_shader_string);
  #ifdef DEBUG
  const auto v_compile_error = getCompileStatus(v_shader);
  if (v_compile_error) {
    log_stream << "vertex shader compilation failed\n";
    log_stream << *v_compile_error << "\n";
  }
  const auto f_compile_error = getCompileStatus(f_shader);
  if (f_compile_error) {
    log_stream << "fragment shader compilation failed\n";
    log_stream << *f_compile_error << "\n";
  }
  #endif

  GLuint shader_program = createProgram(v_shader, f_shader, true);
  #ifdef DEBUG
  const auto link_error = getLinkStatus(shader_program);
  if (link_error) {
    log_stream << "shader program link failed\n";
    log_stream << *link_error << "\n";
  }
  #endif

  glUseProgram(shader_program);

  Rect rect = createRect();

  auto texture_path = xdg::get_data_path(
    base_dirs, "qogl", "textures/wood.jpg"
    #ifdef DEBUG
    , log_stream
    #endif
  );
  Texture texture = loadTexture(
    texture_path->c_str()
    #ifdef DEBUG
    , log_stream
    #endif
  );

  auto [projection, view, model] = fullscreen_rect_matrices(
    window_width, window_height
  );

  uniformMatrix4fv(shader_program, "projection", glm::value_ptr(projection));
  uniformMatrix4fv(shader_program, "view", glm::value_ptr(view));
  uniformMatrix4fv(shader_program, "model", glm::value_ptr(model));

  while (!glfwWindowShouldClose(window)) {
    glClear(GL_COLOR_BUFFER_BIT);
    glfwPollEvents();
    processInput(window);

    glUseProgram(shader_program);
    bindTexture(texture);
    drawRect(rect);

    glfwSwapBuffers(window);
  }

  return 0;
}

void processInput(GLFWwindow *window) {
  if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
    glfwSetWindowShouldClose(window, true);
  }
}
#ifdef DEBUG
std::optional<xdg::path> xdg::get_data_path(
  const xdg::base &b, const std::string &n, const std::string &p,
  fio::log_stream_f &log_stream, const bool create
) {
  log_stream << "Fetching path: " << p << "\n";
  auto path = xdg::get_data_path(b, n, p);
  if (!path) {
    log_stream << "[w] `" << p << "` not found...\n";
  } else {
    log_stream << "--> " << *path << "\n";
  }

  return path;
}

std::optional<xdg::path> fio::read(
  const xdg::path &path, fio::log_stream_f &log_stream
) {
  log_stream << "Loading file: " << path << "\n";
  auto data = fio::read(path);
  if (!data) {
    log_stream << "[w] Could not read file...\n";
  }

  return data;
}

Texture loadTexture(const xdg::path &path, fio::log_stream_f &log_stream) {
  log_stream << "Loading texture: " << path << "\n";

  return loadTexture(path.c_str());
}
#endif

std::array<glm::mat4, 3> fullscreen_rect_matrices(const int w, const int h) {
  glm::mat4 projection = glm::ortho<double>(0, w, 0, h, 0.1, 100.0);

  glm::mat4 view = glm::mat4(1.0);
  view = glm::translate(view, glm::vec3(0.0, 0.0, -1.0));

  glm::mat4 model = glm::mat4(1.0);
  model = glm::scale(model, glm::vec3(w, h, 1));

  return {projection, view, model};
}

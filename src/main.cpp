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

void processInput(GLFWwindow *window);

int main(int argc, const char *argv[]) {
  xdg::base base_dirs = xdg::get_base_directories();

  auto log_path = xdg::get_data_path(base_dirs, "qogl", "logs/qogl.log", true);
  fio::log_stream_f log_stream(*log_path);

  GLFWwindow *window = createWindow(
    gl_major_version, gl_minor_version, true, window_width, window_height,
    "Hello, OpenGL!"
  );

  log_stream << "Attempting to create context: ";
  log_stream << gl_major_version << "." << gl_minor_version << "...\n";

  if (window == nullptr) {
    log_stream << "failed to create window\n";
    glfwDestroyWindow(window);
    return to_underlying(error_code_t::window_failed);
  }

  glfwMakeContextCurrent(window);

  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    log_stream << "failed to initialise GLAD\n";
    return to_underlying(error_code_t::glad_failed);
  }

  {
    std::string opengl_version = reinterpret_cast<const char *>( glGetString(GL_VERSION));
    log_stream << "OpenGL Version: " << opengl_version << "\n";
  }

  {
    std::string glfw_version = glfwGetVersionString();
    log_stream << "GLFW Version: " << glfw_version << "\n";
  }

  glViewport(0, 0, window_width, window_height);
  glClearColor(0.1, 0.1, 0.2, 1.0);

  auto vs_path = xdg::get_data_path(base_dirs, "qogl", "shaders/tex/vshader.glsl");
  if (!vs_path) { log_stream << "vertex shader not found"; }
  auto vs_data = fio::read(*vs_path);
  if (!vs_data) { log_stream << "could not read vertex shader"; }
  std::string v_shader_string = *vs_data;
  log_stream << "loading vertex shader ...\n--> " << *vs_path << "\n";

  auto fs_path = xdg::get_data_path(base_dirs, "qogl", "shaders/tex/fshader.glsl");
  if (!fs_path) { log_stream << "fragment shader not found"; }
  auto fs_data = fio::read(*fs_path);
  if (!fs_data) { log_stream << "could not read fragment shader"; }
  std::string f_shader_string = *fs_data;
  log_stream << "loading fragment shader ...\n--> " << *fs_path << "\n";

  GLuint v_shader = createShader(GL_VERTEX_SHADER, v_shader_string);
  {
    const auto compile_error = getCompileStatus(v_shader);
    if (compile_error) {
      log_stream << "vertex shader compilation failed\n" << *compile_error << "\n";
    }
  }

  GLuint f_shader = createShader(GL_FRAGMENT_SHADER, f_shader_string);
  {
    const auto compile_error = getCompileStatus(f_shader);
    if (compile_error) {
      log_stream << "fragment shader compilation failed\n" << *compile_error << "\n";
    }
  }

  GLuint shader_program = createProgram(v_shader, f_shader, true);
  {
    const auto link_error = getLinkStatus(shader_program);
    if (link_error) {
      log_stream << "shader shader_program link failed\n" << *link_error << "\n";
    }
  }

  glUseProgram(shader_program);

  Rect rect = createRect();

  auto texture_path = xdg::get_data_path(base_dirs, "qogl", "textures/wood.jpg");
  if (!texture_path) { log_stream << "texture not found"; }
  log_stream << "loading texture ...\n--> " << *texture_path << "\n";
  Texture texture = loadTexture(texture_path->c_str());

  glm::mat4 projection = glm::ortho<double>(
    0, window_width,
    0, window_height,
    0.1, 100.0
  );

  glm::mat4 view = glm::mat4(1.0);
  view = glm::translate(view, glm::vec3(0.0, 0.0, -1.0));

  glm::mat4 model = glm::mat4(1.0);
  model = glm::scale(model, glm::vec3(window_width, window_height, 1));

  int projection_loc = glGetUniformLocation(shader_program, "projection");
  glUniformMatrix4fv(projection_loc, 1, GL_FALSE, glm::value_ptr(projection));
  int view_loc = glGetUniformLocation(shader_program, "view");
  glUniformMatrix4fv(view_loc, 1, GL_FALSE, glm::value_ptr(view));
  int model_loc = glGetUniformLocation(shader_program, "model");
  glUniformMatrix4fv(model_loc, 1, GL_FALSE, glm::value_ptr(model));

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

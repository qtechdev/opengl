#include <chrono>
#include <ctime>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "glad.h"
#include <GLFW/glfw3.h>

#include "error.hpp"
#include "file_io.hpp"
#include "shader_program.hpp"
#include "window.hpp"
#include "xdg.hpp"

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

  auto vs_path = xdg::get_data_path(base_dirs, "qogl", "shaders/vshader.glsl");
  if (!vs_path) { log_stream << "vertex shader not found"; }
  auto vs_data = fio::read(*vs_path);
  if (!vs_data) { log_stream << "could not read vertex shader"; }
  std::string v_shader_string = *vs_data;
  log_stream << "loading vertex shader ...\n--> " << *vs_path << "\n";

  auto fs_path = xdg::get_data_path(base_dirs, "qogl", "shaders/fshader.glsl");
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

  std::vector<GLfloat> vertices = {
    /*      a
           /\
        b /__\ c
         /\  /\
        /__\/__\
       d   e   f
    */

     0.0,   0.5, 0.0, // a
    -0.25,  0.0, 0.0, // b
     0.25,  0.0, 0.0, // c
    -0.5,  -0.5, 0.0, // d
     0.0,  -0.5, 0.0, // e
     0.5,  -0.5, 0.0  // f
  };

  std::vector<GLfloat> colours = {
    1.0, 0.0, 0.0, // a
    0.5, 0.5, 0.0, // b
    0.5, 0.0, 0.5, // c
    0.0, 1.0, 0.0, // d
    0.0, 0.5, 0.5, // e
    0.0, 0.0, 1.0  // f
  };

  std::vector<GLuint> indices = {
    0, 1, 2,
    1, 3, 4,
    2, 4, 5
  };

  GLuint vao;
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);

  GLuint vbo;
  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(
    GL_ARRAY_BUFFER, (vertices.size() + colours.size())*sizeof(GLfloat),
    nullptr, GL_STATIC_DRAW
  );
  glBufferSubData(
    GL_ARRAY_BUFFER, 0,
    vertices.size()*sizeof(GLfloat), vertices.data()
  );
  glBufferSubData(
    GL_ARRAY_BUFFER, vertices.size()*sizeof(GLfloat),
    colours.size()*sizeof(GLfloat), colours.data()
  );

  GLuint ebo;
  glGenBuffers(1, &ebo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
  glBufferData(
    GL_ELEMENT_ARRAY_BUFFER, indices.size()*sizeof(GLuint), indices.data(),
    GL_STATIC_DRAW
  );

  glEnableVertexAttribArray(0);
  glVertexAttribPointer(
    0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat),
    (void*)(0)
  );
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(
    1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat),
    (void*)(vertices.size()*sizeof(GLfloat))
  );

  while (!glfwWindowShouldClose(window)) {
    glClear(GL_COLOR_BUFFER_BIT);
    glfwPollEvents();
    processInput(window);

    glUseProgram(shader_program);
    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, 9, GL_UNSIGNED_INT, 0);

    glfwSwapBuffers(window);
  }

  return 0;
}

void processInput(GLFWwindow *window) {
  if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
    glfwSetWindowShouldClose(window, true);
  }
}

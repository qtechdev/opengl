#include <iostream>
#include <string>
#include <vector>

#include "glad.h"
#include <GLFW/glfw3.h>

#include "error.hpp"
#include "file_io.hpp"
#include "shader_program.hpp"
#include "window.hpp"

const int window_width = 640;
const int window_height = 480;

const int gl_major_version = 3;
const int gl_minor_version = 3;

void processInput(GLFWwindow *window);

int main(int argc, const char *argv[]) {
  xdg::base base_dirs = xdg::get_base_directories();
  auto log_path = xdg::get_data_path(base_dirs, "qogl", "logs/qogl.log", true);
  xdg::write(*log_path, "qogl.log ----------------------------------\n", true);

  GLFWwindow *window = createWindow(
    gl_major_version, gl_minor_version, true, window_width, window_height,
    "Hello, OpenGL!"
  );

  {
    std::stringstream gl_version;
    gl_version << gl_major_version << "." << gl_minor_version;
    xdg::write(*log_path, "Attempting to create context: ");
    xdg::write(*log_path, gl_version.str());
    xdg::write(*log_path, "\n");
  }

  if (window == nullptr) {
    xdg::write(*log_path, "failed to create window\n");
    glfwDestroyWindow(window);
    return to_underlying(error_code_t::window_failed);
  }

  glfwMakeContextCurrent(window);

  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    xdg::write(*log_path, "failed to initialise GLAD\n");
    return to_underlying(error_code_t::glad_failed);
  }

  {
    std::string opengl_version = reinterpret_cast<const char *>( glGetString(GL_VERSION));
    xdg::write(*log_path, "OpenGL Version: ");
    xdg::write(*log_path, opengl_version);
    xdg::write(*log_path, "\n");
  }

  {
    std::string glfw_version = glfwGetVersionString();
    xdg::write(*log_path, "GLFW Version: ");
    xdg::write(*log_path, glfw_version);
    xdg::write(*log_path, "\n");
  }

  glViewport(0, 0, window_width, window_height);
  glClearColor(0.1, 0.1, 0.2, 1.0);

  auto vs_path = xdg::get_data_path(base_dirs, "qogl", "shaders/vshader.glsl");
  if (!vs_path) { std::cerr << "vertex shader not found"; }
  auto vs_data = xdg::read(*vs_path);
  if (!vs_data) { std::cerr << "could not read vertex shader"; }
  std::string v_shader_string = *vs_data;
  xdg::write(*log_path, "loading vertex shader ...\n--> ");
  xdg::write(*log_path, *vs_path);
  xdg::write(*log_path, "\n");

  auto fs_path = xdg::get_data_path(base_dirs, "qogl", "shaders/fshader.glsl");
  if (!fs_path) { std::cerr << "fragment shader not found"; }
  auto fs_data = xdg::read(*fs_path);
  if (!fs_data) { std::cerr << "could not read fragment shader"; }
  std::string f_shader_string = *fs_data;
  xdg::write(*log_path, "loading fragment shader ...\n--> ");
  xdg::write(*log_path, *fs_path);
  xdg::write(*log_path, "\n");

  GLuint v_shader = createShader(GL_VERTEX_SHADER, v_shader_string);
  {
    const auto compile_error = getCompileStatus(v_shader);
    if (compile_error) {
      std::cerr << "vertex shader compilation failed\n" << *compile_error << "\n";
    }
  }

  GLuint f_shader = createShader(GL_FRAGMENT_SHADER, f_shader_string);
  {
    const auto compile_error = getCompileStatus(f_shader);
    if (compile_error) {
      std::cerr << "fragment shader compilation failed\n" << *compile_error << "\n";
    }
  }

  GLuint shader_program = createProgram(v_shader, f_shader, true);
  {
    const auto link_error = getLinkStatus(shader_program);
    if (link_error) {
      std::cerr << "shader shader_program link failed\n" << *link_error << "\n";
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

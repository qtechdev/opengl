#include <iostream>
#include <string>
#include <vector>

#include "glad.h"
#include <GLFW/glfw3.h>

#include "error.hpp"
#include "shader_program.hpp"
#include "window.hpp"

const int window_width = 640;
const int window_height = 480;

void processInput(GLFWwindow *window);

int main(int argc, const char *argv[]) {
  GLFWwindow *window = createWindow(
    3, 3, true, window_width, window_height, "Hello, OpenGL!"
  );

  if (window == nullptr) {
    std::cerr << "failed to create window\n";
    glfwDestroyWindow(window);
    return to_underlying(error_code_t::window_failed);
  }

  glfwMakeContextCurrent(window);

  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    std::cerr << "failed to initialise GLAD\n";
    return to_underlying(error_code_t::glad_failed);
  }

  glViewport(0, 0, window_width, window_height);
  glClearColor(0.1, 0.1, 0.2, 1.0);

  std::string v_shader_string = R"vss(
    #version 330 core
    layout (location = 0) in vec3 _pos;

    void main() {
      gl_Position = vec4(_pos.x, _pos.y, _pos.z, 1.0);
    }
  )vss";

  std::string f_shader_string = R"fss(
    #version 330 core
    out vec4 _colour;

    void main() {
      _colour = vec4(1.0, 1.0, 0.5, 1.0);
    }
  )fss";

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

  std::vector<float> vertices = {
    -0.5, -0.5, 0.0,
    0.5, -0.5, 0.0,
    0.0, 0.5, 0.0
  };

  GLuint vao;
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);

  GLuint vbo;
  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, vertices.size()*sizeof(float), vertices.data(), GL_STATIC_DRAW);


  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);


  while (!glfwWindowShouldClose(window)) {
    glClear(GL_COLOR_BUFFER_BIT);
    glfwPollEvents();
    processInput(window);

    glUseProgram(shader_program);
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    glfwSwapBuffers(window);
  }

  return 0;
}

void processInput(GLFWwindow *window) {
  if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
    glfwSetWindowShouldClose(window, true);
  }
}

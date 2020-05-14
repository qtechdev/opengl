#include <iostream>
#include <string>
#include <vector>

#include "glad.h"
#include <GLFW/glfw3.h>

#include "error.hpp"
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

  GLuint v_shader;
  v_shader = glCreateShader(GL_VERTEX_SHADER);
  const GLchar *v_shader_str = v_shader_string.c_str();
  glShaderSource(v_shader, 1, &v_shader_str, nullptr);
  glCompileShader(v_shader);
  {
    int  success;
    char infoLog[512];
    glGetShaderiv(v_shader, GL_COMPILE_STATUS, &success);

    if(!success) {
      glGetShaderInfoLog(v_shader, 512, nullptr, infoLog);
      std::cerr << "vertex shader compilation failed\n" << infoLog << "\n";
    }
  }

  GLuint f_shader;
  f_shader = glCreateShader(GL_FRAGMENT_SHADER);
  const GLchar *f_shader_str = f_shader_string.c_str();
  glShaderSource(f_shader, 1, &f_shader_str, nullptr);
  glCompileShader(f_shader);
  {
    int  success;
    char infoLog[512];
    glGetShaderiv(f_shader, GL_COMPILE_STATUS, &success);

    if(!success) {
      glGetShaderInfoLog(f_shader, 512, nullptr, infoLog);
      std::cerr << "fragment shader compilation failed\n" << infoLog << "\n";
    }
  }

  GLuint shader_program;
  shader_program = glCreateProgram();
  glAttachShader(shader_program, v_shader);
  glAttachShader(shader_program, f_shader);
  glLinkProgram(shader_program);
  {
    int  success;
    char infoLog[512];
    glGetProgramiv(shader_program, GL_LINK_STATUS, &success);

    if(!success) {
      glGetProgramInfoLog(shader_program, 512, nullptr, infoLog);
      std::cerr << "program link failed\n" << infoLog << "\n";
    }
  }

  // glDeleteShader(v_shader);
  // glDeleteShader(f_shader);

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

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
    layout (location = 0) in vec3 attr_pos;
    layout (location = 1) in vec3 attr_colour;

    out vec3 _colour;

    void main() {
      gl_Position = vec4(attr_pos, 1.0);
      _colour = attr_colour;
    }
  )vss";

  std::string f_shader_string = R"fss(
    #version 330 core

    in vec3 _colour;
    out vec4 FragmentColour;

    void main() {
      FragmentColour = vec4(_colour, 1.0);
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

#include <iostream>

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

  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();
    processInput(window);

    glClear(GL_COLOR_BUFFER_BIT);
    glfwSwapBuffers(window);
  }

  return 0;
}

void processInput(GLFWwindow *window) {
  if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
    glfwSetWindowShouldClose(window, true);
  }
}

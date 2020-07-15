#include <algorithm>
#include <array>
#include <chrono>
#include <ctime>
#include <fstream>
#include <iostream>
#include <numeric>
#include <memory>
#include <random>
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
#include "util/timer.hpp"
#include "util/xdg.hpp"

#include "primitives/2d.hpp"

static constexpr int window_width = 640;
static constexpr int window_height = 480;

static constexpr int gl_major_version = 3;
static constexpr int gl_minor_version = 3;

static constexpr timing::seconds physics_timestep(1.0/60.0);
static constexpr timing::seconds output_timestep(1.0);
static constexpr int simulation_speed = 1;
static constexpr int num_aabbs = 450;

int map(const double x, const double n, const double m) {
  return (x / n) * m;
}

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
    base_dirs, "qogl", "shaders/mvp/vshader.glsl"
    #ifdef DEBUG
    , log_stream
    #endif
  );
  auto v_shader_string = fio::read(*v_shader_path);

  auto f_shader_path = xdg::get_data_path(
    base_dirs, "qogl", "shaders/mvp/fshader.glsl"
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

  auto [projection, view, model] = fullscreen_rect_matrices(
    window_width, window_height
  );

  std::random_device rd;
  std::mt19937 engine(rd());
  std::uniform_int_distribution<> x_distribution(10, window_width - 10);
  std::uniform_int_distribution<> y_distribution(10, window_height - 10);
  std::uniform_real_distribution<> mass_disribution(1.0, 1.0);
  std::uniform_real_distribution<> velocity_distribution(-10.0, 10.0);

  std::vector<std::shared_ptr<AABB>> aabbs;
  for (int i = 0; i < num_aabbs; ++i) {
    AABB p;
    p.position = {
      x_distribution(engine),
      y_distribution(engine)
    };
    p.mass = mass_disribution(engine);
    p.velocity = {
      velocity_distribution(engine),
      velocity_distribution(engine)
    };
    p.size = glm::vec2(std::sqrt(p.mass));
    aabbs.push_back(std::make_shared<AABB>(p));
  }

  uniformMatrix4fv(shader_program, "projection", glm::value_ptr(projection));
  uniformMatrix4fv(shader_program, "view", glm::value_ptr(view));

  timing::Clock clock;

  timing::Timer physics_timer;
  timing::seconds physics_accumulator(0.0);
  timing::Timer output_timer;
  timing::seconds output_accumulator(0.0);
  timing::Timer per_frame_timer;
  timing::seconds per_frame_accumulator(0.0);

  int collision_checks = 0;
  std::vector<double> frame_times;

  while (!glfwWindowShouldClose(window)) {
    glClear(GL_COLOR_BUFFER_BIT);
    glfwPollEvents();
    processInput(window);

    glUseProgram(shader_program);

    physics_accumulator += physics_timer.getDelta();
    physics_timer.tick(clock.get());

    while (physics_accumulator >= physics_timestep) {
      for (int i = 0; i < simulation_speed; ++i) {
        per_frame_timer.tick(clock.get());

        for (std::shared_ptr<AABB> p : aabbs) {
          attract(*p, aabbs);
          updateVelocity(*p, physics_timestep.count());
        }

        for (int i = aabbs.size() - 1; i >= 0; --i) {
          AABB p = *aabbs[i];
          auto collisions = checkCollisions(p, aabbs);
          collision_checks += aabbs.size();
          if (collisions) {
            for (std::shared_ptr<AABB> q : *collisions) {
              p.mass += q->mass;
              p.size = glm::vec2(std::sqrt(p.mass));

              double total_mass = p.mass + q->mass;
              p.velocity = (
                (p.velocity * glm::vec2((p.mass / total_mass))) +
                (q->velocity * glm::vec2((q->mass / total_mass)))
              );
              q->is_alive = false;
            }

            auto it = std::remove_if(
              aabbs.begin(), aabbs.end(),
              [](const std::shared_ptr<AABB> p){ return !p->is_alive; }
            );
            aabbs.erase(it, aabbs.end());

            // std::cout << "COLLISION\n";
          }
        }

        for (std::shared_ptr<AABB> p : aabbs) {
          updatePosition(*p, physics_timestep.count());

          // wrap to other edge of screen (world is a torus)
          if (p->position.x < 0) {
            p->position.x = window_width;
          } else if (p->position.x > (window_width)) {
            p->position.x = 0;
          }
          if (p->position.y < 0) {
            p->position.y = window_height;
          } else if (p->position.y > (window_height)) {
            p->position.y = 0;
          }
        }

        per_frame_timer.tick(clock.get());
        frame_times.push_back(per_frame_timer.getDelta().count());
      }

      physics_accumulator -= physics_timestep;
    }


    output_accumulator += output_timer.getDelta();
    output_timer.tick(clock.get());
    while (output_accumulator >= output_timestep) {
      std::cout << "Collision checks: " << collision_checks << "!\n";
      collision_checks = 0;

      double frame_times_sum = std::accumulate(
        frame_times.begin(), frame_times.end(), 0.0
      );
      double avg_frame_time = frame_times_sum / frame_times.size();
      std::cout << "Average frame time: " << avg_frame_time << "!\n";
      frame_times.clear();

      output_accumulator -= output_timestep;
    }

    for (std::shared_ptr<AABB> box : aabbs) {
      glm::mat4 m = glm::mat4(1.0);
      m = glm::translate(m, glm::vec3(box->position, 0.0));
      m = glm::scale(m, glm::vec3(box->size, 0.0));
      uniformMatrix4fv(shader_program, "model", glm::value_ptr(m));
      drawRect(rect);
    }

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

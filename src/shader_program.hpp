#ifndef __SHADER_PROGRAM_HPP__
#define __SHADER_PROGRAM_HPP__
#include <optional>
#include <string>

#include "glad.h"
#include <GLFW/glfw3.h>

GLuint createShader(const GLenum shader_type, const std::string &shader_string);

GLuint createProgram(
  const GLuint v_shader, const GLuint f_shader, const bool delete_shaders=false
);

std::optional<std::string> getCompileStatus(const GLuint shader);
std::optional<std::string> getLinkStatus(const GLuint program);

#endif // __SHADER_PROGRAM_HPP__

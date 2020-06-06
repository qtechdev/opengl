#include "glad.h"
#include <GLFW/glfw3.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "texture.hpp"

Texture loadTexture(const char *texture_path) {
  GLuint texture;
  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  int width;
  int height;
  int channels;
  GLenum fmt;

  stbi_set_flip_vertically_on_load(true);
  unsigned char *data = stbi_load(texture_path, &width, &height, &channels, 0);

  switch (channels) {
    case 1: fmt = GL_RED; break;
    case 4: fmt = GL_RGBA; break;
    default: fmt = GL_RGB;
  }

  glTexImage2D(
    GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, fmt, GL_UNSIGNED_BYTE, data
  );
  stbi_image_free(data);

  glBindTexture(GL_TEXTURE_2D, 0);

  return {texture};
}

void bindTexture(const Texture &t) {
  if (current_texture != t.id) {
    glBindTexture(GL_TEXTURE_2D, t.id);
    current_texture = t.id;
  }
}

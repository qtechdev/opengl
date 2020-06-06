#ifndef __TEXTURE_HPP__
#define __TEXTURE_HPP__
#include "glad.h"
#include <GLFW/glfw3.h>

static GLuint current_texture = 0;

struct Texture {
  GLuint id = 0;
};

Texture loadTexture(const char *path);
void bindTexture(const Texture &t);

#endif // __TEXTURE_HPP__

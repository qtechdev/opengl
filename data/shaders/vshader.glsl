#version 330 core
layout (location = 0) in vec3 attr_pos;
layout (location = 1) in vec3 attr_colour;

out vec3 _colour;

void main() {
  gl_Position = vec4(attr_pos, 1.0);
  _colour = attr_colour;
}

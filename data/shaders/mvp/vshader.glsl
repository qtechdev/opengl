#version 330 core
layout (location = 0) in vec3 attr_pos;
layout (location = 1) in vec3 attr_colour;

out vec3 _colour;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec3 colour_tint;

void main() {
  gl_Position = projection * view * model * vec4(attr_pos, 1.0);
  // gl_Position = projection * view * vec4(attr_pos, 1.0);
  _colour = attr_colour * colour_tint;
}

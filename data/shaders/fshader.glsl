#version 330 core

in vec3 _colour;
out vec4 FragmentColour;

void main() {
  FragmentColour = vec4(_colour, 1.0);
}

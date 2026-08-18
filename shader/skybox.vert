#version 460 core
layout (location = 0) in vec3 aPos;

out vec3 texCoords;

uniform mat4 view;
uniform mat4 projection;

void main() {
  texCoords = aPos;
  vec4 pos = projection * view * vec4(aPos, 1.0);
  /* force depth to the far plane (w/w = 1.0) so the box is always "behind" the scene */
  gl_Position = pos.xyww;
}

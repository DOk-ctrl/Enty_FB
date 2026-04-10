#version 460 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

layout (location = 0) out vec3 normal;
layout (location = 1) out vec2 texCoord;

layout (std140, binding = 0) uniform Matrices {
    mat4 view;
    mat4 projection;
};

uniform mat4 u_Model;

void main() {
  gl_Position = projection * view * u_Model * vec4(aPos, 1.0);
  normal = mat3(transpose(inverse(u_Model))) * aNormal;
  texCoord = aTexCoord;
}


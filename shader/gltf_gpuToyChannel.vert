#version 460 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;
layout (location = 3) in uvec4 aJointNum;
layout (location = 4) in vec4 aJointWeight;

layout (location = 0) out vec3 normal;
layout (location = 1) out vec2 texCoord;
layout (location = 2) out vec3 FragPos; // <--- ƒќЅј¬»Ћ» ¬џ’ќƒ ƒЋя ѕќ«»÷»»

layout (std140, binding = 0) uniform Matrices {
  mat4 view;
  mat4 projection;
};

layout (std430, binding = 1) readonly buffer JointMatrices {
  mat4 jointMat[];
};

uniform int aModelStride;

void main() {
  mat4 skinMat =
    aJointWeight.x * jointMat[int(aJointNum.x) + aModelStride] +
    aJointWeight.y * jointMat[int(aJointNum.y) + aModelStride] +
    aJointWeight.z * jointMat[int(aJointNum.z) + aModelStride] +
    aJointWeight.w * jointMat[int(aJointNum.w) + aModelStride];

  // —читаем мировую позицию вершины
  vec4 worldPos = skinMat * vec4(aPos, 1.0);
  FragPos = vec3(worldPos); // ќтдаем во фрагментный шейдер
  
  gl_Position = projection * view * worldPos;
  
  // ¬ј∆Ќќ: aNormal умножаем на 0.0, так как это вектор направлени€, а не точка в пространстве
  normal = vec3(transpose(inverse(skinMat)) * vec4(aNormal, 0.0));
  texCoord = aTexCoord;
}
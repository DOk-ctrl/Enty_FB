#version 460 core
layout (location = 0) in vec3 aPos;

out vec3 texCoords;

uniform mat4 projection;
uniform mat4 view;
uniform float u_ParallaxAngle;

void main() {
    vec3 pos = aPos;

    // Вращение от параллакса (камеры)
    float cosP = cos(u_ParallaxAngle);
    float sinP = sin(u_ParallaxAngle);
    mat2 rotParallax = mat2(cosP, -sinP, sinP, cosP);
    pos.xz = rotParallax * pos.xz;

    texCoords = pos;

    mat4 staticView = mat4(mat3(view));
    vec4 clipPos = projection * staticView * vec4(aPos, 1.0);
    gl_Position = clipPos.xyww;
}
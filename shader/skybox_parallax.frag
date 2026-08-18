#version 460 core
in  vec3 texCoords;

layout (location = 0) out vec4 fragColor;
layout (location = 1) out vec4 EmitColor;

uniform samplerCube skybox;
uniform float u_Opacity;
uniform int   u_HasRealAlpha;
uniform float u_BrightnessKey;
uniform vec2  u_WindDrift; // Дрейф ветра из C++

void main() {
  vec3 tc = texCoords;

  // 1. Проверяем, смотрит ли пиксель на верхнюю грань (потолок неба)
  // На верхней грани куба Y положительный и больше, чем X и Z
  if (tc.y > abs(tc.x) && tc.y > abs(tc.z)) {
    // Сдвигаем X и Z только для верхней грани (облака плывут по потолку)
    tc.x += u_WindDrift.x;
    tc.z += u_WindDrift.y;
  } 
  // 2. Если это боковые грани (горы/горизонт) — можно слегка поджать Y (если нужно спустить горы)
  else {
    // tc.y = tc.y * 1.5 + 0.3; // Раскомментируй, если надо опустить горы ниже
  }

  vec4 color = texture(skybox, tc);

  if (u_HasRealAlpha == 0) {
    float luma = dot(color.rgb, vec3(0.299, 0.587, 0.114));
    float autoAlpha = 1.0 - smoothstep(u_BrightnessKey, u_BrightnessKey + 0.10, luma);
    color.a = autoAlpha;
  }

  color.a *= u_Opacity;

  if (color.a < 0.02)
    discard;

  fragColor = color;
  EmitColor = vec4(0.0);
}
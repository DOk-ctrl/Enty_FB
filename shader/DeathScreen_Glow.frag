#version 460 core

// »спользует тот же вершинный шейдер, что и Bloom_Blur (fullscreen triangle, gl_VertexID trick)
in  vec2 texCoord;
out vec4 FragColor;

// ѕр€моугольник кнопки в UV-пространстве (0..1, origin Ч низ-лево, как у texCoord)
uniform vec2  u_RectMin;
uniform vec2  u_RectMax;
uniform vec3  u_GlowColor;     // HDR-цвет, напр. (0.3, 0.7, 2.5) Ч синий/белый неон
uniform float u_CornerRadius;  // радиус скруглени€ в UV-единицах
uniform float u_Intensity;     // 0..1, дл€ fade-in / pulse кнопки

float roundedBoxSDF(vec2 p, vec2 halfSize, float r) {
    vec2 d = abs(p) - halfSize + r;
    return length(max(d, 0.0)) - r;
}

void main() {
    vec2 center   = (u_RectMin + u_RectMax) * 0.5;
    vec2 halfSize = (u_RectMax - u_RectMin) * 0.5;
    vec2 p = texCoord - center;

    float dist = roundedBoxSDF(p, halfSize, u_CornerRadius);

    // ¬нутри кнопки Ч полна€ €ркость, м€гкий спад по краю Ч именно этот спад
    // "затравливает" последующий гауссовый блюр красивым ореолом
    float mask = 1.0 - smoothstep(-0.01, 0.02, dist);

    vec3 hdrColor = u_GlowColor * u_Intensity * mask;
    FragColor = vec4(hdrColor, mask);
}

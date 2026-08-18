#version 460 core

in  vec2 texCoord;
out vec4 FragColor;

layout(binding = 0) uniform sampler2D u_GlowBlur;
uniform float u_GlowStrength = 1.0;

void main() {
    vec3 glow = texture(u_GlowBlur, texCoord).rgb * u_GlowStrength;

    // “от же Reinhard + gamma, что и в основном Bloom_Composite.frag Ч
    // чтобы свечение кнопки визуально совпадало по "характеру" с обычным bloom
    glow = glow / (glow + vec3(1.0));
    glow = pow(glow, vec3(1.0 / 2.2));

    // –исуетс€ с glBlendFunc(GL_ONE, GL_ONE) поверх уже собранного кадра Ч
    // альфа не нужна, чистый additive
    FragColor = vec4(glow, 1.0);
}

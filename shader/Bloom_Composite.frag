#version 460 core

in  vec2 texCoord;
out vec4 FragColor;

layout(binding = 0) uniform sampler2D u_Scene;
layout(binding = 1) uniform sampler2D u_BloomBlur;
uniform float u_BloomStrength = 1.0;

void main() {
    // u_Scene уже содержит готовую картинку с тонемаппингом и гаммой из geometry шейдеров
    vec3 scene = texture(u_Scene, texCoord).rgb;

    // Bloom — линейный HDR emission после blur
    // Применяем простой Reinhard чтобы не выбивалось за 1.0, потом гамму
    vec3 bloom = texture(u_BloomBlur, texCoord).rgb * u_BloomStrength;
    bloom = bloom / (bloom + vec3(1.0));
    bloom = pow(bloom, vec3(1.0 / 2.2));

    // Аддитивно добавляем свечение поверх готовой сцены
    vec3 result = scene + bloom;

    FragColor = vec4(result, 1.0);
}

#version 460 core
layout (location = 0) in vec3 normal;
layout (location = 1) in vec2 texCoord;
layout (location = 2) in vec3 FragPos;
layout (location = 3) in mat3 TBN;

out vec4 FragColor;

layout (std140, binding = 0) uniform Matrices {
    mat4 view;
    mat4 projection;
};

uniform sampler2D u_AlbedoMap;   // Слот 0
uniform sampler2D u_NormalMap;   // Слот 1
uniform sampler2D u_MetRoughMap; // Слот 2

uniform int u_HasNormalMap;
uniform int u_HasMetRoughMap;

void main() {
    vec4 texColor = texture(u_AlbedoMap, texCoord);
    
    // 1. Считаем нормаль
    vec3 n;
    if (u_HasNormalMap == 1) {
        vec3 nm = texture(u_NormalMap, texCoord).rgb * 2.0 - 1.0;
        n = normalize(TBN * nm);
    } else {
        n = normalize(normal);
    }

    // 2. Считаем блик (PBR-лайт)
    float roughness = 1.0;
    if (u_HasMetRoughMap == 1) {
        roughness = texture(u_MetRoughMap, texCoord).g; // G канал = Roughness
    }

    // Вектор освещения (твое текущее lightPos)
    vec3 lightPos = vec3(4.0, 5.0, -3.0);
    vec3 lDir = normalize(lightPos - FragPos);
    
    // Диффузное освещение
    float diff = max(dot(n, lDir), 0.0);
    
    // Блик (Specular)
    vec3 viewPos = inverse(view)[3].xyz;
    vec3 vDir = normalize(viewPos - FragPos);
    vec3 hDir = normalize(lDir + vDir);
    float spec = pow(max(dot(n, hDir), 0.0), mix(1.0, 128.0, 1.0 - roughness));
    float specMask = smoothstep(0.4, 0.5, spec) * (1.0 - roughness);

    // Итоговый цвет (твоя формула + блик)
    vec3 ambient = 0.3 * texColor.rgb;
    vec3 diffuse = 0.7 * diff * texColor.rgb;
    vec3 specular = vec3(specMask);

    FragColor = vec4(ambient + diffuse + specular, texColor.a);
}
#version 460 core

layout (location = 0) in vec3 normal;
layout (location = 1) in vec2 texCoord;
layout (location = 2) in vec3 FragPos;

layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 EmitColor; // MRT: нет emission

// --- ТЕКСТУРЫ ---
uniform sampler2D u_BaseColorMap; // Слот 0
uniform sampler2D u_NormalMap;    // Слот 1
uniform sampler2D u_MetallicMap;  // Слот 2
uniform sampler2D u_RoughnessMap; // Слот 3

// --- ОСВЕЩЕНИЕ ---
uniform vec3 u_SunDirection;
uniform vec3 u_SunColor;
uniform float u_SunIntensity;

uniform vec3 u_LightPos;
uniform vec3 u_PointColor;
uniform float u_PointIntensity;

// --- IBL (Image-Based Lighting) maps, generated from the skybox ---
uniform samplerCube u_IrradianceMap; // slot 5
uniform samplerCube u_PrefilterMap;  // slot 6
uniform sampler2D   u_BRDFLUT;       // slot 7

// --- МАТРИЦЫ (нужна для позиции камеры) ---
layout (std140, binding = 0) uniform Matrices {
    mat4 view;
    mat4 projection;
};

const float PI = 3.14159265359;

// 1. ХАК ДЛЯ НОРМАЛЕЙ: Считаем TBN без тангенсов из вершинного шейдера
vec3 getNormalFromMap() {
    vec3 tangentNormal = texture(u_NormalMap, texCoord).xyz * 2.0 - 1.0;
    
    vec3 Q1  = dFdx(FragPos);
    vec3 Q2  = dFdy(FragPos);
    vec2 st1 = dFdx(texCoord);
    vec2 st2 = dFdy(texCoord);
    
    vec3 N   = normalize(normal);
    vec3 T  = normalize(Q1*st2.t - Q2*st1.t);
    vec3 B  = -normalize(cross(N, T));
    mat3 TBN = mat3(T, B, N);
    
    return normalize(TBN * tangentNormal);
}

// 2. МАТЕМАТИКА PBR (Функции GGX и Schlick)
float DistributionGGX(vec3 N, vec3 H, float roughness) {
    float a = roughness*roughness;
    float a2 = a*a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;
    float nom   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
    return nom / max(denom, 0.0000001);
}

float GeometrySchlickGGX(float NdotV, float roughness) {
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;
    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;
    return nom / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);
    return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness) {
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

void main() {
    // Читаем Базовый Цвет (и делаем гамма-коррекцию, так как PBR считает в линейном пространстве)
    vec4 albedoTex = texture(u_BaseColorMap, texCoord);
    if(albedoTex.a < 0.1) discard;
    vec3 albedo = pow(albedoTex.rgb, vec3(2.2));

    // Читаем Металл и Шероховатость
    float metallic = texture(u_MetallicMap, texCoord).r;
    float roughness = texture(u_RoughnessMap, texCoord).r;

    // Получаем итоговую нормаль из карты
    vec3 N = getNormalFromMap();
    
    // Вытаскиваем позицию камеры из inverse view матрицы
    vec3 camPos = vec3(inverse(view)[3]);
    vec3 V = normalize(camPos - FragPos);

    // Базовое отражение (F0). Для диэлектриков берем 0.04, для металлов берем цвет текстуры
    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, albedo, metallic);

    vec3 Lo = vec3(0.0); // Сюда будем плюсовать весь свет

    // --- ИСТОЧНИК 1: СОЛНЦЕ (Directional) ---
    vec3 L_sun = normalize(-u_SunDirection);
    vec3 H_sun = normalize(V + L_sun);
    vec3 radiance_sun = u_SunColor * u_SunIntensity;

    float NDF_sun = DistributionGGX(N, H_sun, roughness);   
    float G_sun   = GeometrySmith(N, V, L_sun, roughness);      
    vec3 F_sun    = fresnelSchlick(max(dot(H_sun, V), 0.0), F0);       

    vec3 numerator_sun    = NDF_sun * G_sun * F_sun; 
    float denominator_sun = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L_sun), 0.0) + 0.0001;
    vec3 specular_sun = numerator_sun / denominator_sun;

    vec3 kS_sun = F_sun;
    vec3 kD_sun = vec3(1.0) - kS_sun;
    kD_sun *= 1.0 - metallic;	  
    
    float NdotL_sun = max(dot(N, L_sun), 0.0);        
    Lo += (kD_sun * albedo / PI + specular_sun) * radiance_sun * NdotL_sun;

    // --- ИСТОЧНИК 2: ЛАМПОЧКА (Point Light) ---
    vec3 L_point = normalize(u_LightPos - FragPos);
    vec3 H_point = normalize(V + L_point);
    float distance = length(u_LightPos - FragPos);
    float attenuation = 1.0 / (distance * distance); // Затухание света
    vec3 radiance_point = u_PointColor * u_PointIntensity * attenuation;

    float NDF_point = DistributionGGX(N, H_point, roughness);   
    float G_point   = GeometrySmith(N, V, L_point, roughness);      
    vec3 F_point    = fresnelSchlick(max(dot(H_point, V), 0.0), F0);       

    vec3 numerator_point    = NDF_point * G_point * F_point; 
    float denominator_point = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L_point), 0.0) + 0.0001;
    vec3 specular_point = numerator_point / denominator_point;

    vec3 kS_point = F_point;
    vec3 kD_point = vec3(1.0) - kS_point;
    kD_point *= 1.0 - metallic;	  
    
    float NdotL_point = max(dot(N, L_point), 0.0);        
    Lo += (kD_point * albedo / PI + specular_point) * radiance_point * NdotL_point;

    // --- ФИНАЛЬНАЯ СБОРКА И ТОНЕМАППИНГ ---
    // Добавляем немного базового "окружающего" света, чтобы тени не были черными как смоль
    // --- IBL AMBIENT (replaces the flat 0.03 constant) ---
    float NdotV = max(dot(N, V), 0.0);
    vec3 F_ambient = fresnelSchlickRoughness(NdotV, F0, roughness);
    vec3 kS_ambient = F_ambient;
    vec3 kD_ambient = (vec3(1.0) - kS_ambient) * (1.0 - metallic);

    vec3 irradiance = texture(u_IrradianceMap, N).rgb;
    vec3 diffuseIBL = irradiance * albedo;

    vec3 R = reflect(-V, N);
    const float MAX_REFLECTION_LOD = 4.0; // mPrefilterMipLevels - 1 (5 mips: 0..4)
    vec3 prefilteredColor = textureLod(u_PrefilterMap, R, roughness * MAX_REFLECTION_LOD).rgb;
    vec2 brdf = texture(u_BRDFLUT, vec2(NdotV, roughness)).rg;
    vec3 specularIBL = prefilteredColor * (F_ambient * brdf.x + brdf.y);

    vec3 ambient = kD_ambient * diffuseIBL + specularIBL;
    vec3 color = ambient + Lo;

    // HDR тонемаппинг (чтобы пересветы уходили в белый плавно)
    color = color / (color + vec3(1.0));
    // Обратная гамма-коррекция для монитора
    color = pow(color, vec3(1.0/2.2)); 

     FragColor = vec4(color, 1.0);
    EmitColor = vec4(0.0); // нет emission
}
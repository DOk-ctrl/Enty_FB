#version 460 core

layout (location = 0) in vec3 normal;
layout (location = 1) in vec2 texCoord;
layout (location = 2) in vec3 FragPos;

// --- MRT Выходы для Блума ---
layout (location = 0) out vec4 FragColor;  // Основная сцена (HDR, линейное пространство)
layout (location = 1) out vec4 EmitColor;  // Карта свечения для Bloom-фильтра

// --- Текстуры ---
uniform sampler2D u_AlbedoMap;       // Слот 0
uniform sampler2D u_NormalMap;       // Слот 1
uniform sampler2D u_PackedMap;       // Слот 2 (R: Metallic, G: Roughness, B: Coat Weight)
uniform sampler2D u_CoatRoughMap;    // Слот 3 (Clearcoat Roughness)
uniform sampler2D u_EmitMap;         // Слот 4 (Emission)

// --- Флаги ---
uniform int u_HasNormalMap;
uniform int u_HasPackedMap;
uniform int u_HasCoatRoughMap;
uniform int u_HasEmitMap;

uniform float u_EmissionMultiplier; // 0.0 = выключено, >0.0 = яркость
// --- Освещение ---
uniform vec3 u_SunDirection;
uniform vec3 u_SunColor;
uniform float u_SunIntensity;

uniform vec3 u_LightPos;
uniform vec3 u_PointColor;
uniform float u_PointIntensity;

// --- IBL (Image-Based Lighting) ---
uniform samplerCube u_IrradianceMap; // Слот 5
uniform samplerCube u_PrefilterMap;  // Слот 6
uniform sampler2D   u_BRDFLUT;       // Слот 7

// --- Матрицы ---
layout (std140, binding = 0) uniform Matrices {
    mat4 view;
    mat4 projection;
};

const float PI = 3.14159265359;

// --- Функции PBR ---
vec3 getNormalFromMap() {
    vec3 tangentNormal = texture(u_NormalMap, texCoord).xyz * 2.0 - 1.0;
    vec3 Q1  = dFdx(FragPos);
    vec3 Q2  = dFdy(FragPos);
    vec2 st1 = dFdx(texCoord);
    vec2 st2 = dFdy(texCoord);
    vec3 N   = normalize(normal);
    vec3 T   = normalize(Q1*st2.t - Q2*st1.t);
    vec3 B   = -normalize(cross(N, T));
    mat3 TBN = mat3(T, B, N);
    return normalize(TBN * tangentNormal);
}

float DistributionGGX(vec3 N, vec3 H, float roughness) {
    float a = roughness*roughness;
    float a2 = a*a;
    float NdotH = max(dot(N, H), 0.0);
    float denom = (NdotH*NdotH * (a2 - 1.0) + 1.0);
    return a2 / (PI * denom * denom);
}

float GeometrySchlickGGX(float NdotV, float roughness) {
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;
    return NdotV / (NdotV * (1.0 - k) + k);
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    return GeometrySchlickGGX(NdotV, roughness) * GeometrySchlickGGX(NdotL, roughness);
}

vec3 fresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness) {
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

vec3 fresnelSchlickClearCoat(float cosTheta, vec3 F0, float coatWeight) {
    return (F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0)) * coatWeight;
}

void main() {
    // 1. Альбедо
    vec4 albedoTex = texture(u_AlbedoMap, texCoord);
    if(albedoTex.a < 0.1) discard;
    vec3 albedo = pow(albedoTex.rgb, vec3(2.2)); // sRGB -> Linear

    // 2. Нормали
    vec3 N = (u_HasNormalMap == 1) ? getNormalFromMap() : normalize(normal);

    // 3. Распаковка Packed текстуры
    float metallic = 0.0;
    float roughness = 0.0;
    float coatWeight = 0.0;
    
    if (u_HasPackedMap == 1) {
        vec3 packedData = texture(u_PackedMap, texCoord).rgb;
        metallic = packedData.r;
        roughness = packedData.g;
        coatWeight = packedData.b;
    }

    float coatRoughness = 0.05;
    if (u_HasCoatRoughMap == 1) {
        coatRoughness = texture(u_CoatRoughMap, texCoord).r;
    }

    coatRoughness = clamp(coatRoughness, 0.01, 1.0);

    // Вектора
    vec3 camPos = vec3(inverse(view)[3]);
    vec3 V = normalize(camPos - FragPos);

    vec3 F0_base = mix(vec3(0.04), albedo, metallic);
    vec3 F0_coat = vec3(0.04); 

    vec3 Lo = vec3(0.0);

    float NdotV = max(dot(N, V), 0.0);

    // =======================================================
    // Направление 1: Солнце
    // =======================================================
    {
        vec3 L_sun = normalize(-u_SunDirection);
        vec3 H_sun = normalize(V + L_sun);
        vec3 radiance_sun = u_SunColor * u_SunIntensity;
        float NdotL_sun = max(dot(N, L_sun), 0.0);

        // CLEARCOAT
        float NDF_coat_sun = DistributionGGX(N, H_sun, coatRoughness);   
        float G_coat_sun   = GeometrySmith(N, V, L_sun, coatRoughness);      
        vec3 F_coat_sun    = fresnelSchlickClearCoat(max(dot(H_sun, V), 0.0), F0_coat, coatWeight);       
        vec3 spec_coat_sun = (NDF_coat_sun * G_coat_sun * F_coat_sun) / (4.0 * NdotV * NdotL_sun + 0.0001);

        // BASE
        float NDF_base_sun = DistributionGGX(N, H_sun, roughness);   
        float G_base_sun   = GeometrySmith(N, V, L_sun, roughness);      
        vec3 F_base_sun    = fresnelSchlick(max(dot(H_sun, V), 0.0), F0_base);       
        vec3 spec_base_sun = (NDF_base_sun * G_base_sun * F_base_sun) / (4.0 * NdotV * NdotL_sun + 0.0001);

        vec3 kD_sun = (vec3(1.0) - F_base_sun) * (1.0 - metallic);
        vec3 baseLobe_sun = (kD_sun * albedo / PI) + spec_base_sun;

        Lo += (spec_coat_sun + baseLobe_sun * (vec3(1.0) - F_coat_sun)) * radiance_sun * NdotL_sun;
    }

    // =======================================================
    // Направление 2: Точечный свет
    // =======================================================
    {
        vec3 L_point = normalize(u_LightPos - FragPos);
        vec3 H_point = normalize(V + L_point);
        float dist = length(u_LightPos - FragPos);
        float attenuation = 1.0 / (dist * dist);
        vec3 radiance_point = u_PointColor * u_PointIntensity * attenuation;
        float NdotL_point = max(dot(N, L_point), 0.0);

        // CLEARCOAT
        float NDF_coat_pt = DistributionGGX(N, H_point, coatRoughness);   
        float G_coat_pt   = GeometrySmith(N, V, L_point, coatRoughness);      
        vec3 F_coat_pt    = fresnelSchlickClearCoat(max(dot(H_point, V), 0.0), F0_coat, coatWeight);       
        vec3 spec_coat_pt = (NDF_coat_pt * G_coat_pt * F_coat_pt) / (4.0 * NdotV * NdotL_point + 0.0001);

        // BASE
        float NDF_base_pt = DistributionGGX(N, H_point, roughness);   
        float G_base_pt   = GeometrySmith(N, V, L_point, roughness);      
        vec3 F_base_pt    = fresnelSchlick(max(dot(H_point, V), 0.0), F0_base);       
        vec3 spec_base_pt = (NDF_base_pt * G_base_pt * F_base_pt) / (4.0 * NdotV * NdotL_point + 0.0001);

        vec3 kD_point = (vec3(1.0) - F_base_pt) * (1.0 - metallic);
        vec3 baseLobe_pt = (kD_point * albedo / PI) + spec_base_pt;

        Lo += (spec_coat_pt + baseLobe_pt * (vec3(1.0) - F_coat_pt)) * radiance_point * NdotL_point;
    }

    // =======================================================
    // Свечение (Emission)
    // =======================================================
    vec3 emission = vec3(0.0);
    if (u_HasEmitMap == 1) {
        emission = texture(u_EmitMap, texCoord).rgb * u_EmissionMultiplier; 
    }

    // =======================================================
    // IBL AMBIENT
    // =======================================================
    vec3 F_ambient = fresnelSchlickRoughness(NdotV, F0_base, roughness);
    vec3 kS_ambient = F_ambient;
    vec3 kD_ambient = (vec3(1.0) - kS_ambient) * (1.0 - metallic);

    vec3 irradiance = texture(u_IrradianceMap, N).rgb;
    vec3 diffuseIBL = irradiance * albedo;

    vec3 R = reflect(-V, N);
    const float MAX_REFLECTION_LOD = 4.0; 
    vec3 prefilteredColor = textureLod(u_PrefilterMap, R, roughness * MAX_REFLECTION_LOD).rgb;
    vec2 brdf = texture(u_BRDFLUT, vec2(NdotV, roughness)).rg;
    vec3 specularIBL = prefilteredColor * (F_ambient * brdf.x + brdf.y);

    vec3 ambient = kD_ambient * diffuseIBL + specularIBL;
    
    // Финальный цвет в HDR (Линейное пространство)
    vec3 finalColor = ambient + Lo + emission;

    // Запись в MRT-буферы
    FragColor = vec4(finalColor, albedoTex.a);
    EmitColor = vec4(emission, 1.0); 
}
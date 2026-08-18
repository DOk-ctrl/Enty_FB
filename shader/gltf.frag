#version 460 core

layout (location = 0) in vec3 normal;
layout (location = 1) in vec2 texCoord;
layout (location = 2) in vec3 FragPos;

// MRT: два выхода
layout (location = 0) out vec4 FragColor;  // COLOR0 — HDR сцена (без тонемаппинга!)
layout (location = 1) out vec4 EmitColor;  // COLOR1 — emission для bloom

uniform sampler2D u_BaseColorMap;
uniform sampler2D u_NormalMap;
uniform sampler2D u_MetallicMap;
uniform sampler2D u_RoughnessMap;
uniform sampler2D u_EmitMap;
uniform int u_HasEmitMap;
uniform int u_HasNormalMap;
uniform int u_HasMetalMap;
uniform int u_HasRoughMap;

uniform vec3  u_SunDirection;
uniform vec3  u_SunColor;
uniform float u_SunIntensity;
uniform vec3  u_LightPos;
uniform vec3  u_PointColor;
uniform float u_PointIntensity;

uniform float u_EmitStrength; // множитель emission (например 2.0)

uniform samplerCube u_IrradianceMap;
uniform samplerCube u_PrefilterMap;
uniform sampler2D   u_BRDFLUT;

layout (std140, binding = 0) uniform Matrices {
    mat4 view;
    mat4 projection;
};

const float PI = 3.14159265359;

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
    float a  = roughness * roughness;
    float a2 = a * a;
    float NdotH  = max(dot(N, H), 0.0);
    float denom  = (NdotH * NdotH * (a2 - 1.0) + 1.0);
    return a2 / (PI * denom * denom);
}

float GeometrySchlickGGX(float NdotV, float roughness) {
    float r = roughness + 1.0;
    float k = (r * r) / 8.0;
    return NdotV / (NdotV * (1.0 - k) + k);
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    return GeometrySchlickGGX(max(dot(N, V), 0.0), roughness)
         * GeometrySchlickGGX(max(dot(N, L), 0.0), roughness);
}

vec3 fresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness) {
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

void main() {
    vec4 albedoTex = texture(u_BaseColorMap, texCoord);
    if (albedoTex.a < 0.1) discard;
    vec3 albedo = pow(albedoTex.rgb, vec3(2.2)); // sRGB → linear

    vec3  N         = (u_HasNormalMap == 1) ? getNormalFromMap() : normalize(normal);
    float metallic  = (u_HasMetalMap  == 1) ? texture(u_MetallicMap,  texCoord).r : 0.0;
    float roughness = (u_HasRoughMap  == 1) ? texture(u_RoughnessMap, texCoord).r : 0.8;

    vec3 camPos = vec3(inverse(view)[3]);
    vec3 V  = normalize(camPos - FragPos);
    vec3 F0 = mix(vec3(0.04), albedo, metallic);
    vec3 Lo = vec3(0.0);

    // --- Солнце ---
    {
        vec3  L = normalize(-u_SunDirection);
        vec3  H = normalize(V + L);
        vec3  rad = u_SunColor * u_SunIntensity;
        float NDF = DistributionGGX(N, H, roughness);
        float G   = GeometrySmith(N, V, L, roughness);
        vec3  F   = fresnelSchlick(max(dot(H, V), 0.0), F0);
        vec3  spec = (NDF * G * F) / (4.0 * max(dot(N,V),0.0) * max(dot(N,L),0.0) + 0.0001);
        vec3  kD  = (vec3(1.0) - F) * (1.0 - metallic);
        Lo += (kD * albedo / PI + spec) * rad * max(dot(N, L), 0.0);
    }

    // --- Точечный свет ---
    {
        vec3  L   = normalize(u_LightPos - FragPos);
        vec3  H   = normalize(V + L);
        float dist = length(u_LightPos - FragPos);
        vec3  rad = u_PointColor * u_PointIntensity / (dist * dist);
        float NDF = DistributionGGX(N, H, roughness);
        float G   = GeometrySmith(N, V, L, roughness);
        vec3  F   = fresnelSchlick(max(dot(H, V), 0.0), F0);
        vec3  spec = (NDF * G * F) / (4.0 * max(dot(N,V),0.0) * max(dot(N,L),0.0) + 0.0001);
        vec3  kD  = (vec3(1.0) - F) * (1.0 - metallic);
        Lo += (kD * albedo / PI + spec) * rad * max(dot(N, L), 0.0);
    }

    // --- IBL ambient ---
    float NdotV = max(dot(N, V), 0.0);
    vec3  F_amb = fresnelSchlickRoughness(NdotV, F0, roughness);
    vec3  kD_amb = (vec3(1.0) - F_amb) * (1.0 - metallic);
    vec3  irradiance = texture(u_IrradianceMap, N).rgb;
    vec3  R = reflect(-V, N);
    vec3  prefiltered = textureLod(u_PrefilterMap, R, roughness * 4.0).rgb;
    vec2  brdf = texture(u_BRDFLUT, vec2(NdotV, roughness)).rg;
    vec3  ambient = kD_amb * irradiance * albedo + prefiltered * (F_amb * brdf.x + brdf.y);

    // HDR цвет сцены (линейный, без тонемаппинга и гаммы — это делает composite)
    vec3 hdrColor = ambient + Lo;

    // --- Emission ---
    vec3 emission = vec3(0.0);
    if (u_HasEmitMap == 1) {
        emission = texture(u_EmitMap, texCoord).rgb * u_EmitStrength;
    }

    // COLOR0: HDR сцена + emission (тонемаппинг делается в composite шейдере)
    FragColor = vec4(hdrColor + emission, albedoTex.a);

    // COLOR1: только emission для bloom (линейный HDR, без гаммы)
    EmitColor = vec4(emission, 1.0);
}

#version 460 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;
layout (location = 3) in ivec4 aJoints;  // Совпадает с {"JOINTS_0", 3}
layout (location = 4) in vec4 aWeights; // Совпадает with {"WEIGHTS_0", 4}
layout (location = 5) in vec4 aTangent; // Совпадает with {"TANGENT", 5}

layout (location = 0) out vec3 normal;
layout (location = 1) out vec2 texCoord;
layout (location = 2) out vec3 FragPos;
layout (location = 3) out mat3 TBN;

layout (std140, binding = 0) uniform Matrices {
    mat4 view;
    mat4 projection;
};

uniform mat4 u_Model;

void main() {
    FragPos = vec3(u_Model * vec4(aPos, 1.0));
    gl_Position = projection * view * vec4(FragPos, 1.0);
    texCoord = aTexCoord;

    mat3 normalMatrix = transpose(inverse(mat3(u_Model)));
    vec3 T = normalize(normalMatrix * aTangent.xyz);
    vec3 N = normalize(normalMatrix * aNormal);
    T = normalize(T - dot(T, N) * N);
    vec3 B = cross(N, T) * aTangent.w;

    TBN = mat3(T, B, N);
    normal = N;
}
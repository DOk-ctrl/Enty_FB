#version 460 core

in vec3 texCoords;
layout (location = 0) out vec4 fragColor;
layout (location = 1) out vec4 EmitColor; // MRT: скайбокс не светится

uniform samplerCube skybox;

void main() {
    fragColor  = texture(skybox, texCoords);
    EmitColor  = vec4(0.0);
}

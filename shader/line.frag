#version 460 core

layout (location = 0) in vec4 lineColor;
layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 EmitColor; // MRT: линии не светятся

void main() {
    FragColor = lineColor;
    EmitColor = vec4(0.0);
}

#version 460 core
layout (location = 0) in vec3 normal;
layout (location = 1) in vec2 texCoord;

layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 EmitColor; // MRT: всегда пишем 0, emit нет

uniform sampler2D tex;

void main() {
    vec4 texColor = texture(tex, texCoord);
    if(texColor.a < 0.1)
        discard;

    FragColor = texColor * 1.1;
    EmitColor = vec4(0.0); // нет emission Ч €вно обнул€ем
}

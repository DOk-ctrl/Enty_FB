#version 460 core
layout (location = 0) in vec3 normal;
layout (location = 1) in vec2 texCoord;

out vec4 FragColor;

uniform sampler2D tex;
vec3 lightPos = vec3(4.0, 3.0, 6.0);
vec3 lightColor = vec3(1.0, 1.0, 1.0);

void main() {

   vec4 texColor = texture(tex, texCoord);
    
    if(texColor.a < 0.1)
        discard;
    
    FragColor = texColor * 1.1; // Ёто сделает персонажа идеально €рким везде
}
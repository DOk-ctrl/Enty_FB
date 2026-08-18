#version 460 core

in  vec2 texCoord;
out vec4 FragColor;

uniform sampler2D u_Image;
uniform int u_Horizontal; // 1 = горизонтальный, 0 = вертикальный (bool ненадёжен через glUniform1i)

const float weight[5] = float[](0.227027, 0.194595, 0.121622, 0.054054, 0.016216);

void main() {
    vec2 texelSize = 1.0 / textureSize(u_Image, 0);
    vec3 result = texture(u_Image, texCoord).rgb * weight[0];

    if (u_Horizontal == 1) {
        for (int i = 1; i < 5; ++i) {
            result += texture(u_Image, texCoord + vec2(texelSize.x * float(i), 0.0)).rgb * weight[i];
            result += texture(u_Image, texCoord - vec2(texelSize.x * float(i), 0.0)).rgb * weight[i];
        }
    } else {
        for (int i = 1; i < 5; ++i) {
            result += texture(u_Image, texCoord + vec2(0.0, texelSize.y * float(i))).rgb * weight[i];
            result += texture(u_Image, texCoord - vec2(0.0, texelSize.y * float(i))).rgb * weight[i];
        }
    }

    FragColor = vec4(result, 1.0);
}

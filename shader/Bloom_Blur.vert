#version 460 core

// Fullscreen triangle trick: без VBO, просто gl_VertexID
// Draw call: glDrawArrays(GL_TRIANGLES, 0, 3)
out vec2 texCoord;

void main() {
    vec2 positions[3] = vec2[](
        vec2(-1.0, -1.0),
        vec2( 3.0, -1.0),
        vec2(-1.0,  3.0)
    );
    vec2 coords[3] = vec2[](
        vec2(0.0, 0.0),
        vec2(2.0, 0.0),
        vec2(0.0, 2.0)
    );
    texCoord = coords[gl_VertexID];
    gl_Position = vec4(positions[gl_VertexID], 0.0, 1.0);
}

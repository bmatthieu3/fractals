#version 330 core
layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texcoord;
layout (location = 3) in ivec4 id;
layout (location = 4) in vec4 weights;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec2 txc;

void main() {
    gl_Position = projection * view * model * vec4(pos, 1.0);
    txc = texcoord;
}
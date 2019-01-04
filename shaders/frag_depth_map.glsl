#version 330 core
out vec4 color;
in vec2 tx;

uniform sampler2D tex_screen;

void main() {
    color = vec4(vec3(texture(tex_screen, tx).r), 1.f);
}
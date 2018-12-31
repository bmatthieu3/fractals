#version 330 core
out vec4 color;
in vec2 txc;

uniform float time;

uniform sampler2D tex_diffuse1;
uniform sampler2D tex_specular1;

void main() {
    color = texture(tex_diffuse1, txc);
    //color = vec4(1, 0, 0, 1);
}
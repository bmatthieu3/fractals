#version 330 core
out vec4 color;
in vec2 txc;

uniform float time;

uniform sampler2D tex_diffuse;
uniform sampler2D tex_specular;

void main() {
    color = texture2D(tex_diffuse, txc);
    //color = vec4(1, 0, 0, 1);
}
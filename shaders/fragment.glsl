#version 330 core
out vec4 color;
in vec2 txc;

uniform float time;

uniform sampler2D tex_diffuse1;
uniform sampler2D tex_specular1;

void main() {
    float lambda = sin(3*time)/2 + 0.5f;
    color = mix(texture(tex_diffuse1, txc).rgba, texture(tex_specular1, txc).rgba, lambda);
    //color = vec4(1, 0, 0, 1);
}
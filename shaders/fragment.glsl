#version 330 core
out vec4 color;
in vec2 txc;

uniform float time;

uniform sampler2D texdata;

void main()
{
    color = texture(texdata, txc);
} 
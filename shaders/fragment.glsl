#version 330 core
out vec4 FragColor;

uniform float time;

void main()
{
    float red = (sin(time*5.f)/2.f) + 0.5f;
    FragColor = vec4(red, 0.f, 0.f, 1.0f);
} 
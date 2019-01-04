#version 330 core
layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texcoord;

// Uniform local to the mesh being rendered
uniform mat4 model;
uniform mat4 nModel;
uniform mat4 view;
uniform mat4 projection;

// Global uniforms
uniform mat4 viewLightSpace;
uniform mat4 clipLightSpace;

out vec2 tx;
out vec4 posLightSpace;
out vec3 posWorldSpace;
out vec3 normalWorldSpace;

void main() {
    gl_Position = projection * view * model * vec4(pos, 1.0);
    tx = texcoord;
    posLightSpace = clipLightSpace * viewLightSpace * model * vec4(pos, 1.0);
    normalWorldSpace = vec3(nModel * vec4(normal, 1.0));
    posWorldSpace = vec3(model * vec4(pos, 1.0));
}
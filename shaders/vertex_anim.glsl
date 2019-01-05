#version 330 core
layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texcoord;
layout (location = 3) in vec3 tangent;
layout (location = 4) in vec3 bitangent;
layout (location = 5) in ivec4 id;
layout (location = 6) in vec4 weights;

// Uniform local to the mesh being rendered
uniform mat4 model;
uniform mat4 nModel;

// Global uniforms
uniform float time;
uniform mat4 view;
uniform mat4 projection;

uniform mat4 viewLightSpace;
uniform mat4 clipLightSpace;

uniform vec3 eyeWorldSpace;

struct DirectionalLight {
    vec3 ambiant;
    vec3 diffuse;
    vec3 specular;

    vec3 dir;
};
uniform DirectionalLight sun;

out vec2 tx;
out vec4 posLightSpace;

out vec3 posFragLocalSpace;
out vec3 posEyeLocalSpace;
out vec3 lightDirectionLocalSpace;
out mat3 TBNLocalToWorld;

void main() {
    gl_Position = projection * view * model * vec4(pos, 1.0);
    tx = texcoord;
    vec4 posWorldSpace = model * vec4(pos, 1.f);
    posLightSpace = clipLightSpace * viewLightSpace * posWorldSpace;
    
    vec3 normalWorldSpace = normalize(vec3(nModel * vec4(normal, 0.0)));
    vec3 tangentWorldSpace = normalize(vec3(nModel * vec4(tangent, 0.0)));
    // re-orthogonalize T with respect to N
    //tangentWorldSpace = normalize(tangentWorldSpace - dot(tangentWorldSpace, normalWorldSpace) * normalWorldSpace);
    // then retrieve perpendicular vector B with the cross product of T and N
    //vec3 bitangentWorldSpace = cross(normalWorldSpace, tangentWorldSpace);
    vec3 bitangentWorldSpace = normalize(vec3(nModel * vec4(bitangent, 0.0)));

    TBNLocalToWorld = mat3(tangentWorldSpace, bitangentWorldSpace, normalWorldSpace);
    mat3 TBNWorldToLocal = transpose(TBNLocalToWorld);

    lightDirectionLocalSpace = TBNWorldToLocal * (-sun.dir);
    posEyeLocalSpace = TBNWorldToLocal * eyeWorldSpace;
    posFragLocalSpace = TBNWorldToLocal * posWorldSpace.xyz;
}
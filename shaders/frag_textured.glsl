#version 330 core
out vec4 color;

in vec4 posLightSpace;
in vec3 posWorldSpace;

in vec2 tx;
in vec3 normalWorldSpace;

uniform float time;
uniform int screen_w;
uniform int screen_h;
uniform vec3 eyePositionWorldSpace;

uniform sampler2D depth_map;

struct Material {
    float shininess;
};

uniform Material material;

struct DirectionalLight {
    vec3 ambiant;
    vec3 diffuse;
    vec3 specular;

    vec3 dir;
};

uniform DirectionalLight sun;

uniform sampler2D diffuse_map;
uniform sampler2D specular_map;
uniform sampler2D normal_map;

vec4 applyShadow(vec4 lightColor, vec4 shadowColor) {
    // Change the pos in light space to Normalized Device Coordinates (between -1 and 1)
    vec3 posLightNDCSpace = posLightSpace.xyz / posLightSpace.w;
    vec3 depthTx = posLightNDCSpace * 0.5f + 0.5f;
    float currentFragDepth = depthTx.z;
    
    float shadowFactor = 0.f;
    // PCF with a 3x3 kernel
    vec2 texelSize = vec2(1.f / screen_w, 1.f / screen_h);
    for(int x = -1; x <= 1; x++) {
        for(int y = -1; y <= 1; y++) {
            float storedDepth = texture(depth_map, depthTx.xy + texelSize * vec2(x, y)).r;
            shadowFactor += (storedDepth + 0.001 < currentFragDepth) ? 1.f : 0.f;
        }
    }
    shadowFactor /= 9.f;

    return mix(lightColor, shadowColor, shadowFactor);
}

void main() {
    // Normalize after the GPU's interpolation of normal
    vec3 N = normalize(normalWorldSpace);
    vec3 L = -sun.dir;
    vec3 V = normalize(eyePositionWorldSpace - posWorldSpace);

    vec3 H = normalize(L + V);

    float spec = max(pow(dot(N, H), 4.f * material.shininess), 0);
    float diff = max(dot(L, N), 0.f);
    
    vec3 ambiantColor = sun.ambiant * texture(diffuse_map, tx).rgb;
    vec3 diffuseColor = sun.diffuse * diff * texture(diffuse_map, tx).rgb;
    vec3 specularColor = sun.specular * spec * texture(specular_map, tx).rgb;

    vec4 lightColor = vec4(ambiantColor + diffuseColor + specularColor, 1.0);
    vec4 shadowColor = vec4(lightColor.xyz * 0.5, 1.f);

    color = applyShadow(lightColor, shadowColor); 
}
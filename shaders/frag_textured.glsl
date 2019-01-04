#version 330 core
out vec4 color;
in vec4 posLightSpace;
in vec2 tx;

uniform float time;
uniform int screen_w;
uniform int screen_h;

uniform sampler2D depth_map;

uniform sampler2D diffuse_map;
uniform sampler2D specular_map;

vec4 applyShadow(vec4 lightColor, vec4 shadowColor) {
    // Change the pos in light space to Normalized Device Coordinates (between -1 and 1)
    vec3 posLightNDCSpace = posLightSpace.xyz / posLightSpace.w;
    vec3 depthTx = posLightNDCSpace * 0.5f + 0.5f;
    float currentFragDepth = depthTx.z;
    
    float shadowFactor = 0.f;
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
    vec4 shadowColor = vec4(0.f, 0.f, 0.f, 1.f);
    vec4 lightColor = texture(diffuse_map, tx);
    color = applyShadow(lightColor, shadowColor); 
}
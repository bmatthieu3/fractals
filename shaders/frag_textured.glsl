#version 330 core
out vec4 color;

in vec4 posLightSpace;
in vec2 tx;
//in vec3 posFragLocalSpace;
in vec4 posWorldSpace;
//in vec3 posEyeLocalSpace;
//in vec3 lightDirectionLocalSpace;
in mat3 TBNLocalToWorld;

// Local uniforms
struct Material {
    float shininess;
};

uniform Material material;

// Global uniforms
uniform float time;
uniform int screen_w;
uniform int screen_h;
uniform vec3 eyeWorldSpace;

uniform sampler2D depth_map;

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

vec4 applyShadow(vec4 lightColor, vec4 shadowColor, vec3 normal) {
    // Change the pos in light space to Normalized Device Coordinates (between [-1, 1])
    vec3 posLightNDCSpace = posLightSpace.xyz / posLightSpace.w;
    // Mapping to [0, 1]
    vec3 depthTx = posLightNDCSpace * 0.5f + 0.5f;
    // Store the depth of the current fragment to test
    float currentFragDepth = depthTx.z;

    float bias = max(0.002, 0.004 - abs(dot(sun.dir, normal)));

    float shadowFactor = 0.f;
    // PCF with a 3x3 kernel
    vec2 texelSize = vec2(1.f / screen_w, 1.f / screen_h);
    for(int x = -1; x <= 1; x++) {
        for(int y = -1; y <= 1; y++) {
            float storedDepth = texture(depth_map, depthTx.xy + texelSize * vec2(x, y)).r;
            shadowFactor += (storedDepth + bias < currentFragDepth) ? 1.f : 0.f;
        }
    }
    shadowFactor /= 9.f;

    return mix(lightColor, shadowColor, shadowFactor);
}

void main() {
    // Normalize after the GPU's interpolation of normal
    vec3 normalLocalSpace = texture(normal_map, tx).rgb;
    normalLocalSpace = normalize(2*normalLocalSpace - 1);
    vec3 N = normalize(TBNLocalToWorld * normalLocalSpace);

    vec3 L = normalize(-sun.dir);
    vec3 V = normalize(eyeWorldSpace - vec3(posWorldSpace));
    vec3 H = normalize(L + V);

    float diff = max(dot(L, N), 0.f);
    float spec = max(pow(dot(N, H), 4.f * material.shininess), 0);
    
    vec3 ambiantColor = sun.ambiant * texture(diffuse_map, tx).rgb;
    vec3 diffuseColor = sun.diffuse * diff * texture(diffuse_map, tx).rgb;
    vec3 specularColor = sun.specular * spec * texture(specular_map, tx).rgb;

    float K = 10;
    vec4 lightColor = K*vec4(ambiantColor + diffuseColor + specularColor, 1.0);
    vec4 shadowColor = vec4(vec3(lightColor*0.05), 1.f);
    //color = lightColor;
    color = applyShadow(lightColor, shadowColor, N); 
    //color = vec4(N*0.5 + 0.5, 1.f);

    // Basic HDR
    //color = color / (color + vec4(1.f));
    float exposure = 1.f;
    color = 1.f - (exp(-color*exposure));
}
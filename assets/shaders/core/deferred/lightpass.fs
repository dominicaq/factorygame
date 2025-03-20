#version 430 core

// Input/Output
in vec2 TexCoords;
in vec3 FragPos;
out vec4 FragColor;

// G-buffer textures
uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedo;

// Camera data
uniform vec3 u_CameraPosition;

// SSBO for lights
struct Light {
    vec3 position; int _pad1;
    vec3 color; int _pad2;
    float radius;
    float intensity;
    uint  depthHandle;
    int isPointLight;
};

layout(std430, binding = 0) buffer LightBuffer {
    Light lights[];
};

uniform int numLights; // Number of active lights

// Material properties
const float AMBIENT_STRENGTH = 0.7;
const float DIFFUSE_STRENGTH = 0.3;
const float SPECULAR_STRENGTH = 0.8;
const float SPECULAR_SHININESS = 16.0;

void main() {
    vec3 worldPos = texture(gPosition, TexCoords).rgb;
    vec3 normal = normalize(texture(gNormal, TexCoords).rgb);
    vec3 albedo = texture(gAlbedo, TexCoords).rgb;
    vec3 viewDir = normalize(u_CameraPosition - worldPos);

    vec3 ambient = AMBIENT_STRENGTH * albedo;
    vec3 lighting = ambient;

    for (int i = 0; i < numLights; ++i) {
        Light light = lights[i];
        vec3 lightVec = light.position - worldPos;
        float distance = length(lightVec);
        vec3 lightDir = lightVec / distance;

        // Apply attenuation and clamp it
        float attenuation = max(0.0, 1.0 - distance / light.radius);
        attenuation = clamp(attenuation, 0.0, 1.0);

        if (attenuation <= 0.0) continue;

        // Diffuse lighting
        float diffuseFactor = max(dot(normal, lightDir), 0.0);
        vec3 diffuse = diffuseFactor * light.color * light.intensity * DIFFUSE_STRENGTH;

        // Specular lighting
        vec3 reflectDir = reflect(-lightDir, normal);
        float specularFactor = pow(max(dot(viewDir, reflectDir), 0.0), SPECULAR_SHININESS);
        vec3 specular = light.color * specularFactor * light.intensity * SPECULAR_STRENGTH;

        lighting += attenuation * (diffuse + specular);
    }

    // Final color output
    FragColor = vec4(lighting * albedo, 1.0);
}

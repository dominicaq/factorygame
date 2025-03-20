#version 430

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
    vec3 position; float radius;                      // 16 bytes
    vec3 color; float intensity;                      // 16 bytes
    int isPointLight; int castShadow; int pad_2; int pad_3; // 16 bytes
    mat4 lightSpaceMatrices[6];                      // 96 bytes (used for point lights)
};

layout(std430, binding = 0) buffer LightBuffer {
    Light lights[];
};

uniform int numLights;
uniform int shadowWidth;

// Shadow maps (array of samplers)
uniform sampler2D shadowMaps[30];  // Up to 6 shadow maps for point lights

// Material properties
const float AMBIENT_STRENGTH = 0.7;
const float DIFFUSE_STRENGTH = 0.3;
const float SPECULAR_STRENGTH = 0.8;
const float SPECULAR_SHININESS = 16.0;

// Function to calculate shadow by sampling all 6 faces for point lights
float calculateShadow(vec3 fragPos, Light light, int shadowMapIndex) {
    if (light.castShadow == 0) return 0.0; // No shadow

    vec3 lightToFrag = fragPos - light.position;
    float bias = 0.025;

    if (light.isPointLight == 1) {
        // Find the dominant face direction
        vec3 absVec = abs(lightToFrag);
        float maxComponent = max(max(absVec.x, absVec.y), absVec.z);

        int faceIndex;
        mat4 lightMatrix;

        // Determine which face to use based on the dominant direction
        if (absVec.x == maxComponent) {
            faceIndex = (lightToFrag.x > 0.0) ? 0 : 1; // +X or -X
        } else if (absVec.y == maxComponent) {
            faceIndex = (lightToFrag.y > 0.0) ? 2 : 3; // +Y or -Y
        } else {
            faceIndex = (lightToFrag.z > 0.0) ? 4 : 5; // +Z or -Z
        }

        lightMatrix = light.lightSpaceMatrices[faceIndex];

        // Transform fragment position to light space
        vec4 fragPosLightSpace = lightMatrix * vec4(fragPos, 1.0);
        vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
        projCoords = projCoords * 0.5 + 0.5; // Convert to [0,1] range

        // Skip if outside visible range
        if (projCoords.x < 0.0 || projCoords.x > 1.0 ||
            projCoords.y < 0.0 || projCoords.y > 1.0 ||
            projCoords.z < 0.0 || projCoords.z > 1.0) {
            return 0.0;
        }

        // Calculate UV coordinates for the horizontal atlas
        float faceWidth = 1.0 / 6.0;
        vec2 atlasCoords = vec2(
            projCoords.x * faceWidth + faceIndex * faceWidth,
            projCoords.y
        );

        // Sample depth from the atlas
        float closestDepth = texture(shadowMaps[shadowMapIndex], atlasCoords).r;
        return (projCoords.z - bias > closestDepth) ? 1.0 : 0.0;
    } else {
        // Non-point light shadow (directional/spot)
        vec4 fragPosLightSpace = light.lightSpaceMatrices[0] * vec4(fragPos, 1.0);
        vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
        projCoords = projCoords * 0.5 + 0.5;

        if (projCoords.z < 0.0 || projCoords.z > 1.0) return 0.0;

        float closestDepth = texture(shadowMaps[shadowMapIndex], projCoords.xy).r;
        return (projCoords.z - bias > closestDepth) ? 1.0 : 0.0;
    }
}

void main() {
    // Sample data from G-buffer
    vec3 worldPos = texture(gPosition, TexCoords).rgb;
    vec3 normal = normalize(texture(gNormal, TexCoords).rgb);
    vec3 albedo = texture(gAlbedo, TexCoords).rgb;
    vec3 viewDir = normalize(u_CameraPosition - worldPos);

    // Calculate ambient light
    vec3 ambient = AMBIENT_STRENGTH * albedo;
    vec3 lighting = ambient;

    // Process each light
    for (int i = 0; i < numLights; ++i) {
        Light light = lights[i];
        vec3 lightVec = light.position - worldPos;
        float distance = length(lightVec);
        vec3 lightDir = normalize(lightVec);

        // Apply distance attenuation
        float attenuation = max(0.0, 1.0 - distance / light.radius);
        attenuation = attenuation * attenuation;

        if (attenuation <= 0.0) continue;

        // Diffuse lighting
        float diffuseFactor = max(dot(normal, lightDir), 0.0);
        vec3 diffuse = diffuseFactor * light.color * light.intensity * DIFFUSE_STRENGTH;

        // Specular lighting
        vec3 halfwayDir = normalize(lightDir + viewDir);
        float specularFactor = pow(max(dot(normal, halfwayDir), 0.0), SPECULAR_SHININESS);
        vec3 specular = light.color * specularFactor * light.intensity * SPECULAR_STRENGTH;

        // Calculate shadow for this light
        float shadow = calculateShadow(worldPos, light, i);

        // Apply shadow to lighting
        vec3 lightContribution = attenuation * (diffuse + specular) * (1.0 - shadow);
        lighting += lightContribution;
    }

    // Final color output
    FragColor = vec4(lighting * albedo, 1.0);
}

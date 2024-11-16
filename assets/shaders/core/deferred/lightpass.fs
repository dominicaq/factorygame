#version 330 core

in vec2 TexCoords;
in vec3 FragPos;
out vec4 FragColor;

// G-buffer textures
uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedo;

// Shadow atlas data
uniform sampler2D u_ShadowAtlas;
uniform int u_AtlasSize;
uniform int u_TileSize;

// Scene Data
uniform vec3 u_CameraPosition;

// Debug toggle for shadows
bool u_DebugPitchBlackShadows = true;

// Light structure
struct Light {
    vec3 position;
    vec3 color;
    float intensity;
    int atlasIndices[6];
    mat4 lightSpaceMatrices[6];
};

#define MAX_LIGHTS 10
uniform int numLights;
uniform Light lights[MAX_LIGHTS];

// Function to compute shadow contribution for a single face
float SampleShadow(vec4 lightSpaceFragPos, int tileIndex) {
    // Perform perspective divide
    vec3 projCoords = lightSpaceFragPos.xyz / lightSpaceFragPos.w;

    // Map to [0, 1] UV space
    projCoords = projCoords * 0.5 + 0.5;

    // Check if out of bounds
    if (projCoords.x < 0.0 || projCoords.x > 1.0 || projCoords.y < 0.0 || projCoords.y > 1.0 || projCoords.z > 1.0) {
        return 1.0; // Not in shadow
    }

    // Compute shadow atlas UV
    int tilesPerRow = u_AtlasSize / u_TileSize;
    vec2 tileOffset = vec2(tileIndex % tilesPerRow, tileIndex / tilesPerRow) * float(u_TileSize) / float(u_AtlasSize);
    vec2 atlasUV = projCoords.xy * float(u_TileSize) / float(u_AtlasSize) + tileOffset;

    // Sample shadow map and perform depth comparison
    float shadowDepth = texture(u_ShadowAtlas, atlasUV).r;
    float currentDepth = projCoords.z;

    // Add a bias to avoid shadow acne
    float bias = 0.005;
    return currentDepth - bias > shadowDepth ? 0.0 : 1.0;
}

void main() {
    // Retrieve data from G-buffer
    vec3 FragPos = texture(gPosition, TexCoords).rgb;
    vec3 Normal = normalize(texture(gNormal, TexCoords).rgb);
    vec3 Albedo = texture(gAlbedo, TexCoords).rgb;

    // Ambient component
    vec3 ambient = 0.2 * Albedo;
    vec3 lighting = ambient;

    // Accumulate lighting for each light
    for (int i = 0; i < numLights; ++i) {
        Light light = lights[i];

        vec3 lightDir = normalize(light.position - FragPos);

        // Diffuse component
        float diff = max(dot(Normal, lightDir), 0.0);
        vec3 diffuse = diff * light.color * light.intensity * 0.8;

        // Specular component
        vec3 viewDir = normalize(u_CameraPosition - FragPos);
        vec3 reflectDir = reflect(-lightDir, Normal);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
        vec3 specular = light.color * spec * light.intensity * 0.5;

        // Shadow calculation
        float shadow = 0.0;
        for (int j = 0; j < 6; ++j) {
            if (light.atlasIndices[j] == -1) {
                break;
            }
            vec4 lightSpaceFragPos = light.lightSpaceMatrices[j] * vec4(FragPos, 1.0);
            shadow += SampleShadow(lightSpaceFragPos, light.atlasIndices[j]);
        }
        shadow /= 6.0;

        // Darken the shadow based on debug mode
        if (u_DebugPitchBlackShadows) {
            shadow = shadow < 1.0 ? 0.0 : 1.0; // Make shadowed regions pitch black
        } else {
            shadow = mix(0.2, 1.0, shadow); // Normal shadow rendering
        }

        // Combine lighting with shadow
        lighting += shadow * (diffuse + specular);
    }

    // Blend albedo and lighting
    FragColor = vec4(mix(Albedo, lighting, 0.5), 1.0);
}

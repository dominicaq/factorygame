#version 330 core

// Input/Output
in vec2 TexCoords;
in vec3 FragPos;
out vec4 FragColor;

// G-buffer textures
uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedo;

// Shadow atlas configuration
uniform sampler2D u_ShadowAtlas;
uniform int u_AtlasSize;
uniform int u_TileSize;

// Camera data
uniform vec3 u_CameraPosition;

// Shadow debug options
uniform bool u_DebugPitchBlackShadows = true;

// Light structure definition
struct Light {
    vec3 position;
    vec3 color;
    float intensity;
    float radius;
    bool castsShadows;
    int atlasIndices[6];        // Shadow map tile indices for cubemap faces
    mat4 lightSpaceMatrices[6]; // View-projection matrices for cubemap faces
};

// Light array
#define BATCH_SIZE 16
uniform int numLights;
uniform Light lights[BATCH_SIZE];

// Material properties - keeping the original values
const float AMBIENT_STRENGTH = 0.4;
const float DIFFUSE_STRENGTH = 0.6;
const float SPECULAR_STRENGTH = 0.5;
const float SPECULAR_SHININESS = 4.0;
const float SHADOW_BIAS = 0.005;
const float MIN_SHADOW_VISIBILITY = 0.2;

/**
 * Calculates shadow factor for a fragment from a specific light direction
 * @param lightSpaceFragPos Fragment position in light space
 * @param tileIndex Index of the shadow map tile in the atlas
 * @return Shadow factor (0.0 = full shadow, 1.0 = no shadow)
 */
float SampleShadow(vec4 lightSpaceFragPos, int tileIndex) {
    // Perspective divide to get normalized device coordinates
    vec3 projCoords = lightSpaceFragPos.xyz / lightSpaceFragPos.w;

    // Transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;

    // Check if fragment is outside the light's view frustum
    if (projCoords.x < 0.0 || projCoords.x > 1.0 ||
        projCoords.y < 0.0 || projCoords.y > 1.0 ||
        projCoords.z > 1.0) {
        return 1.0; // Not in shadow if outside frustum
    }

    // Calculate tile position in shadow atlas
    int tilesPerRow = u_AtlasSize / u_TileSize;
    vec2 tileOffset = vec2(tileIndex % tilesPerRow, tileIndex / tilesPerRow) *
                     float(u_TileSize) / float(u_AtlasSize);

    // Convert fragment coordinates to atlas UV coordinates
    vec2 atlasUV = projCoords.xy * float(u_TileSize) / float(u_AtlasSize) + tileOffset;

    // Get depth from shadow map - use explicit component to ensure we're getting depth
    float shadowDepth = texture(u_ShadowAtlas, atlasUV).r;
    float currentDepth = projCoords.z;

    // Debug visualization
    // Uncomment to debug the shadow map sampling
    // if (shadowDepth < 0.99) {
    //     return 0.5; // Visualize any non-empty shadowmap areas
    // }

    // Use a larger bias for depth comparison to fix shadow acne
    float adjustedBias = SHADOW_BIAS;
    // Slope-scaled bias can help with angled surfaces
    // float nDotL = dot(normal, normalize(light.position - worldPos));
    // adjustedBias = max(0.005 * (1.0 - nDotL), SHADOW_BIAS);

    // Compare depths with bias to avoid shadow acne
    return currentDepth - adjustedBias > shadowDepth ? 0.0 : 1.0;
}

void main() {
    // Get G-buffer data
    vec3 worldPos = texture(gPosition, TexCoords).rgb;
    vec3 normal = normalize(texture(gNormal, TexCoords).rgb);
    vec3 albedo = texture(gAlbedo, TexCoords).rgb;

    // Calculate view direction
    vec3 viewDir = normalize(u_CameraPosition - worldPos);

    // Initialize with ambient lighting
    vec3 ambient = AMBIENT_STRENGTH * albedo;
    vec3 lighting = ambient;

    // Debug flag to isolate shadow rendering issues
    bool debugShadowsOnly = false;

    // Process each light
    for (int i = 0; i < numLights; ++i) {
        Light light = lights[i];
        vec3 lightVec = light.position - worldPos;
        float distance = length(lightVec);
        vec3 lightDir = lightVec / distance; // Normalize

        // Calculate attenuation based on radius
        float attenuation = max(0.0, 1.0 - distance / light.radius);
        attenuation = attenuation * attenuation; // Quadratic falloff for more realistic light

        // Skip calculations if fragment is outside light radius
        if (attenuation <= 0.0) continue;

        // Calculate diffuse lighting
        float diffuseFactor = max(dot(normal, lightDir), 0.0);
        vec3 diffuse = diffuseFactor * light.color * light.intensity * DIFFUSE_STRENGTH;

        // Calculate specular lighting (Phong model)
        vec3 reflectDir = reflect(-lightDir, normal);
        float specularFactor = pow(max(dot(viewDir, reflectDir), 0.0), SPECULAR_SHININESS);
        vec3 specular = light.color * specularFactor * light.intensity * SPECULAR_STRENGTH;

        // Calculate shadow factor - default to no shadow
        float shadow = 1.0;

        // Only calculate shadows if this light casts them
        if (light.castsShadows) {
            // CRITICAL FIX: Ensure we're using the light indices correctly
            // Check if at least one valid face exists
            bool hasValidFace = false;
            for (int j = 0; j < 6; ++j) {
                if (light.atlasIndices[j] >= 0) {
                    hasValidFace = true;
                    break;
                }
            }

            if (!hasValidFace) {
                // If no valid faces, don't cast shadows
                shadow = 1.0;
            } else {
                // Reset shadow calculation for valid faces
                shadow = 0.0;
                int validFaces = 0;

                for (int j = 0; j < 6; ++j) {
                    if (light.atlasIndices[j] < 0) {
                        continue; // Skip invalid atlas indices (-1)
                    }

                    // Transform position to light space for this face
                    vec4 lightSpacePos = light.lightSpaceMatrices[j] * vec4(worldPos, 1.0);

                    // CRITICAL FIX: Check if this face can see the fragment
                    // For cubemap faces, only one face should be able to see the fragment
                    // This avoids incorrect averaging of shadow contributions
                    vec3 lightToFrag = normalize(worldPos - light.position);

                    // Determine which face this direction corresponds to
                    // For cubemap: +X = 0, -X = 1, +Y = 2, -Y = 3, +Z = 4, -Z = 5
                    int faceMask = -1;
                    vec3 absDir = abs(lightToFrag);
                    float maxComp = max(max(absDir.x, absDir.y), absDir.z);

                    if (maxComp == absDir.x) {
                        faceMask = lightToFrag.x > 0.0 ? 0 : 1;
                    } else if (maxComp == absDir.y) {
                        faceMask = lightToFrag.y > 0.0 ? 2 : 3;
                    } else {
                        faceMask = lightToFrag.z > 0.0 ? 4 : 5;
                    }

                    // Only sample from the correct face
                    if (j == faceMask) {
                        shadow += SampleShadow(lightSpacePos, light.atlasIndices[j]);
                        validFaces++;
                    }
                }

                // Average the shadow contributions from valid faces
                shadow = validFaces > 0 ? shadow / float(validFaces) : 1.0;
            }

            // Apply shadow visualization mode
            if (u_DebugPitchBlackShadows) {
                // Hard shadows for debugging
                shadow = shadow < 1.0 ? 0.0 : 1.0;
            } else {
                // Soft shadows for normal rendering
                shadow = mix(MIN_SHADOW_VISIBILITY, 1.0, shadow);
            }

            // Debug shadows only
            if (debugShadowsOnly) {
                lighting = vec3(shadow);
                continue;  // Skip the rest of lighting calculations
            }
        }

        // Apply attenuation to light contribution
        lighting += shadow * attenuation * (diffuse + specular);
    }

    // For shadow debugging, just output the shadow map directly
    // Uncomment this to debug shadow atlas
    // FragColor = vec4(texture(u_ShadowAtlas, TexCoords).rrr, 1.0);
    // return;

    // Combine lighting with albedo
    FragColor = vec4(lighting * albedo, 1.0);
}

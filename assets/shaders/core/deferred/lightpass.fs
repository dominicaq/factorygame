#version 430

// Input/Output
in vec2 TexCoords;
in vec3 FragPos;
out vec4 FragColor;

// G-buffer textures
uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedo;
uniform sampler2D gPBRParams;

// Camera data
uniform vec3 u_CameraPosition;

// SSBO for lights
struct Light {
    vec3 position; float radius; // 16 bytes
    vec3 color; float intensity; // 16 bytes
    int isPointLight; int castShadow; int lightMatrixIndex; int shadowMapIndex; // 16 bytes
};

layout(std430, binding = 0) buffer LightBuffer {
    Light lights[];
};

// SSBO for light matrices
layout(std430, binding = 1) buffer LightMatrixBuffer {
    mat4 lightMatrices[];
};

// Light data uniforms
uniform int numLights;
uniform sampler2D shadowMaps[20];

// Constants
const float PI = 3.14159265359;

// Function to calculate shadow by sampling all 6 faces for point lights
float calculateShadow(vec3 fragPos, Light light) {
    if (light.castShadow == 0) return 0.0; // No shadow

    vec3 lightToFrag = fragPos - light.position;
    vec3 normal = normalize(texture(gNormal, TexCoords).rgb);
    vec3 lightDir = normalize(light.position - fragPos);

    // Dynamically adjusted bias to reduce shadow acne and peter-panning
    float bias = clamp(0.005 * tan(acos(dot(normal, lightDir))), 0.001, 0.03);

    if (light.isPointLight == 1) {
        // For point lights, we need to calculate which face of the cubemap we are on
        vec3 absVec = abs(lightToFrag);
        float maxComponent = max(max(absVec.x, absVec.y), absVec.z);

        int faceIndex;

        // Select the cubemap face based on the dominant axis of the light-to-fragment vector
        if (absVec.x == maxComponent) {
            faceIndex = (lightToFrag.x > 0.0) ? 0 : 1; // +X or -X
        } else if (absVec.y == maxComponent) {
            faceIndex = (lightToFrag.y > 0.0) ? 2 : 3; // +Y or -Y
        } else {
            faceIndex = (lightToFrag.z > 0.0) ? 4 : 5; // +Z or -Z
        }

        // Use the correct matrix for the selected face
        mat4 lightMatrix = lightMatrices[light.lightMatrixIndex + faceIndex];

        // Transform to light space
        vec4 fragPosLightSpace = lightMatrix * vec4(fragPos, 1.0);
        vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
        projCoords = projCoords * 0.5 + 0.5; // Convert to [0,1] range

        if (projCoords.x < 0.0 || projCoords.x > 1.0 ||
            projCoords.y < 0.0 || projCoords.y > 1.0 ||
            projCoords.z < 0.0 || projCoords.z > 1.0) {
            return 0.0;
        }

        // Calculate face width with inset padding to avoid bleeding
        float faceWidth = 1.0 / 6.0;

        // Add a small inset to avoid sampling across face boundaries
        float inset = 0.001; // Small value to move sampling away from edges

        // Scale down the UV coordinates within each face to avoid edge sampling
        float scaledX = projCoords.x * (1.0 - 2.0 * inset) + inset;

        // Calculate atlas coordinates with padding between faces
        vec2 atlasCoords = vec2(
            scaledX * faceWidth + faceIndex * faceWidth,
            projCoords.y
        );

        // PCF Sampling - ensure we don't sample across face boundaries
        float shadowFactor = 0.0;
        int kernelSize = 3;
        float samples = float(kernelSize * kernelSize);
        vec2 texelSize = 1.0 / vec2(textureSize(shadowMaps[light.shadowMapIndex], 0));

        // Calculate the maximum allowed offset to stay within face boundaries
        float maxOffsetX = faceWidth * 0.5 - inset * 2.0;

        for (int x = -1; x <= 1; ++x) {
            for (int y = -1; y <= 1; ++y) {
                // Limit the X offset to stay within current face
                float limitedOffsetX = clamp(float(x) * texelSize.x, -maxOffsetX, maxOffsetX);
                vec2 offset = vec2(limitedOffsetX, float(y) * texelSize.y);

                float depthSample = texture(shadowMaps[light.shadowMapIndex], atlasCoords + offset).r;
                shadowFactor += (projCoords.z - bias > depthSample) ? 1.0 : 0.0;
            }
        }

        shadowFactor /= samples;
        return shadowFactor;
    } else {
        // Directional/spot lights - use the correct matrix index
        mat4 lightMatrix = lightMatrices[light.lightMatrixIndex];
        vec4 fragPosLightSpace = lightMatrix * vec4(fragPos, 1.0);
        vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
        projCoords = projCoords * 0.5 + 0.5;

        if (projCoords.z < 0.0 || projCoords.z > 1.0) return 0.0;

        float shadowFactor = 0.0;
        int kernelSize = 3;
        float samples = float(kernelSize * kernelSize);
        vec2 texelSize = 1.0 / vec2(textureSize(shadowMaps[light.shadowMapIndex], 0));

        for (int x = -1; x <= 1; ++x) {
            for (int y = -1; y <= 1; ++y) {
                vec2 offset = vec2(x, y) * texelSize;
                float depthSample = texture(shadowMaps[light.shadowMapIndex], projCoords.xy + offset).r;
                shadowFactor += (projCoords.z - bias > depthSample) ? 1.0 : 0.0;
            }
        }

        shadowFactor /= samples;
        return shadowFactor;
    }
}

// PBR functions
float DistributionGGX(vec3 N, vec3 H, float roughness) {
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float nom = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / max(denom, 0.001);
}

float GeometrySchlickGGX(float NdotV, float roughness) {
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float nom = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / max(denom, 0.001);
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(max(1.0 - cosTheta, 0.0), 5.0);
}

void main() {
    // Sample data from G-buffer
    vec3 worldPos = texture(gPosition, TexCoords).rgb;
    vec3 normal = normalize(texture(gNormal, TexCoords).rgb);
    vec3 albedo = texture(gAlbedo, TexCoords).rgb;

    // Sample PBR parameters
    vec4 pbrParams = texture(gPBRParams, TexCoords);
    float metallic = pbrParams.r;
    float roughness = pbrParams.g;
    float ao = pbrParams.b;

    vec3 viewDir = normalize(u_CameraPosition - worldPos);

    // Prepare variables for PBR
    vec3 F0 = vec3(0.04); // Default base reflectivity for dielectrics
    F0 = mix(F0, albedo, metallic); // Adjust for metals

    // Calculate ambient light
    vec3 ambient = vec3(0.03) * albedo * ao;
    vec3 lighting = ambient;

    // Process each light
    for (int i = 0; i < numLights; ++i) {
        Light light = lights[i];
        vec3 lightVec = light.position - worldPos;
        float distance = length(lightVec);
        vec3 lightDir = normalize(lightVec);

        // Calculate attenuation
        float attenuation = max(0.0, 1.0 - distance / light.radius);
        attenuation = attenuation * attenuation;

        if (attenuation <= 0.0) continue;

        // Calculate shadow for this light
        float shadow = calculateShadow(worldPos, light);

        // Skip further calculations if fully in shadow
        if (shadow >= 1.0) continue;

        // PBR calculations
        vec3 halfwayDir = normalize(viewDir + lightDir);
        float NdotL = max(dot(normal, lightDir), 0.0);

        // Skip if light is behind surface
        if (NdotL <= 0.0) continue;

        // Cook-Torrance BRDF
        float NDF = DistributionGGX(normal, halfwayDir, roughness);
        float G = GeometrySmith(normal, viewDir, lightDir, roughness);
        vec3 F = fresnelSchlick(max(dot(halfwayDir, viewDir), 0.0), F0);

        vec3 kS = F; // Specular contribution
        vec3 kD = vec3(1.0) - kS; // Diffuse contribution
        kD *= 1.0 - metallic; // Metallic surfaces have no diffuse

        vec3 numerator = NDF * G * F;
        float denominator = 4.0 * max(dot(normal, viewDir), 0.0) * NdotL + 0.001;
        vec3 specular = numerator / denominator;

        // Add light contribution
        vec3 radiance = light.color * light.intensity * attenuation;
        vec3 lightContribution = (kD * albedo / PI + specular) * radiance * NdotL;

        // Apply shadow
        lighting += lightContribution * (1.0 - shadow);
    }

    // Apply tone mapping (simple Reinhard operator)
    // lighting = lighting / (lighting + vec3(1.0));

    // Gamma correction
    // lighting = pow(lighting, vec3(1.0/2.2));

    // Final color output
    FragColor = vec4(lighting, 1.0);
}

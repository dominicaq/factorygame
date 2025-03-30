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

// Skybox texture
uniform samplerCube skybox;

// SSBO for lights
struct PointLight {
    vec3 position; float radius;
    vec3 color; float intensity;
    int castShadow;  int shadowMapIndex; int lightMatrixIndex; int _padding;
};

struct SpotLight {
    vec3 position; float innerCutoff;
    vec3 direction; float outerCutoff;
    vec3 color; float intensity;
    float range; int castShadow; int shadowMapIndex; int lightMatrixIndex;
};

struct DirectionalLight {
    vec3 dir; float shadowOrthoSize;
    vec3 color; float intensity;
    int castShadow; int shadowMapIndex; int lightMatrixIndex;
};

layout(std430, binding = 0) buffer PointLightBuffer {
    PointLight pointLights[];
};

layout(std430, binding = 1) buffer SpotLightBuffer {
    SpotLight spotLights[];
};

// SSBO for light matrices
layout(std430, binding = 2) buffer LightMatrixBuffer {
    mat4 lightMatrices[];
};

// Light data
uniform int numDirectionalLights;
uniform int numPointLights;
uniform int numSpotLights;
uniform DirectionalLight directionalLights[3];
uniform sampler2D shadowMaps[20];

// Constants
const float PI = 3.14159265359;

float calculatePointShadow(vec3 fragPos, PointLight light) {
    if (light.castShadow == 0) return 0.0; // No shadow

    vec3 lightToFrag = fragPos - light.position;
    vec3 normal = normalize(texture(gNormal, TexCoords).rgb);
    vec3 lightDir = normalize(light.position - fragPos);

    // Dynamically adjusted bias to reduce shadow acne and peter-panning
    float bias = clamp(0.005 * tan(acos(dot(normal, lightDir))), 0.001, 0.03);

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
}

float calculateSpotShadow(vec3 fragPos, SpotLight light) {
    if (light.castShadow == 0) return 0.0; // No shadow

    vec3 normal = normalize(texture(gNormal, TexCoords).rgb);
    vec3 lightDir = normalize(light.position - fragPos);

    // Calculate bias based on surface angle to light and distance
    float NdotL = max(dot(normal, lightDir), 0.0);

    // Use a smaller base bias to reduce peter panning
    float baseBias = 0.0005;

    // Scale bias based on the angle between surface normal and light direction
    // This helps with surfaces at grazing angles where shadow acne is more visible
    float angleFactor = 1.0 - NdotL;

    // Calculate distance from fragment to light
    float lightDistance = length(light.position - fragPos);

    // Increase bias slightly with distance to compensate for shadow map resolution
    float distanceFactor = lightDistance / light.range;

    // Final bias calculation with much smaller range
    float bias = baseBias * (1.0 + angleFactor * 5.0 + distanceFactor);

    // Clamp to a much smaller maximum value to reduce peter panning
    bias = clamp(bias, 0.0001, 0.005);

    // Transform fragment position to light space using the light's view-projection matrix
    mat4 lightMatrix = lightMatrices[light.lightMatrixIndex];
    vec4 fragPosLightSpace = lightMatrix * vec4(fragPos, 1.0);

    // Perspective division
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;

    // Transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;

    // Check if fragment is outside the light's view frustum
    if (projCoords.x < 0.0 || projCoords.x > 1.0 ||
        projCoords.y < 0.0 || projCoords.y > 1.0 ||
        projCoords.z < 0.0 || projCoords.z > 1.0) {
        return 0.0;
    }

    // PCF Sampling with optimizations
    float shadowFactor = 0.0;
    int kernelSize = 3;
    vec2 texelSize = 1.0 / vec2(textureSize(shadowMaps[light.shadowMapIndex], 0));

    // Get depth from shadow map once to avoid redundant texture fetches
    float currentDepth = projCoords.z;

    // Use hardware PCF if available (gives better performance)
    for (int x = -1; x <= 1; ++x) {
        for (int y = -1; y <= 1; ++y) {
            vec2 offset = vec2(float(x), float(y)) * texelSize;
            float depthSample = texture(shadowMaps[light.shadowMapIndex], projCoords.xy + offset).r;
            shadowFactor += (currentDepth - bias > depthSample) ? 1.0 : 0.0;
        }
    }

    shadowFactor /= 9.0;

    // Add depth-based blending to gradually fade shadows at their edges
    // This creates more natural shadow transitions
    float shadowDistance = abs(currentDepth - texture(shadowMaps[light.shadowMapIndex], projCoords.xy).r);
    float shadowEdgeFactor = smoothstep(0.0, bias * 2.0, shadowDistance);

    // Apply subtle shadow strength adjustment based on distance
    // Shadows become slightly weaker at distance for a more realistic look
    float shadowStrength = mix(1.0, 0.85, distanceFactor);

    return shadowFactor * shadowStrength;
}

// PBR functions - unchanged
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

// Physically accurate skybox sampling function that respects material properties
vec3 sampleSkybox(samplerCube skybox, vec3 reflectionVector, float roughness) {
    // Use appropriate mip level based on roughness - physically accurate
    float mipLevel = roughness * 5.0;
    return textureLod(skybox, reflectionVector, mipLevel).rgb;
}

// Unified PBR lighting calculation function
vec3 calculatePBRLighting(
    vec3 worldPos, vec3 normal, vec3 viewDir, vec3 lightDir, vec3 lightColor,
    float lightIntensity, float attenuation, vec3 albedo, float metallic, float roughness, vec3 F0, float shadow
) {
    // Skip if light is behind surface
    float NdotL = max(dot(normal, lightDir), 0.0);
    if (NdotL <= 0.0) return vec3(0.0);

    // PBR calculations
    vec3 halfwayDir = normalize(viewDir + lightDir);

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

    // Calculate light contribution
    vec3 radiance = lightColor * lightIntensity * attenuation;
    vec3 lightContribution = (kD * albedo / PI + specular) * radiance * NdotL;

    // Apply shadow
    return lightContribution * (1.0 - shadow);
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

    // Calculate ambient light contribution - keep it dark for dark materials
    // Use a very low constant ambient value to preserve darkness
    vec3 baseAmbient = vec3(0.02) * albedo * ao;

    // Calculate reflection vector for skybox sampling
    vec3 reflectionVector = reflect(-viewDir, normal);

    // Sample skybox using physically-based method
    vec3 skyboxColor = sampleSkybox(skybox, reflectionVector, roughness);

    // Calculate realistic fresnel effect for reflections
    float fresnelFactor = pow(1.0 - max(0.0, dot(normal, viewDir)), 5.0);

    // For metals, use albedo as reflection color. For dielectrics, use white
    vec3 reflectionColor = mix(vec3(1.0), albedo, metallic);

    // Calculate reflection contribution - physical PBR model
    // Metals reflect based on their color, dielectrics reflect skybox directly
    // Rougher surfaces reflect less (diffuse more)
    vec3 skyboxContribution = skyboxColor * reflectionColor;

    // Calculate physically-based reflection strength
    // Higher for metals, lower for dielectrics
    // Higher for smooth surfaces, lower for rough surfaces
    // Higher at grazing angles (fresnel effect)
    float baseReflectivity = mix(0.04, 0.9, metallic); // Dielectrics ~4%, metals ~90%
    float reflectionStrength = mix(baseReflectivity, 1.0, fresnelFactor);
    reflectionStrength *= (1.0 - roughness * roughness); // Squared for physical correctness

    // Combine ambient and skybox reflection - physically-based mix
    vec3 ambient = mix(baseAmbient, skyboxContribution, reflectionStrength);

    // Preserve dark areas by ensuring ambient doesn't get too bright
    float ambientLuma = dot(ambient, vec3(0.299, 0.587, 0.114));
    float maxAmbient = 0.5 * (1.0 - ao); // Darker for occluded areas
    if (ambientLuma > maxAmbient) {
        ambient *= maxAmbient / ambientLuma;
    }

    // Start with the ambient light (now physically based with preserved darks)
    vec3 lighting = ambient;

    // Process each point light
    for (int i = 0; i < numPointLights; ++i) {
        PointLight light = pointLights[i];
        vec3 lightVec = light.position - worldPos;
        float distance = length(lightVec);
        vec3 lightDir = normalize(lightVec);

        // Calculate attenuation
        float attenuation = max(0.0, 1.0 - distance / light.radius);
        attenuation = attenuation * attenuation;

        if (attenuation <= 0.0) continue;

        // Calculate shadow for this light
        float shadow = calculatePointShadow(worldPos, light);

        // Skip further calculations if fully in shadow
        if (shadow >= 1.0) continue;

        // Calculate and add point light contribution using the unified function
        lighting += calculatePBRLighting(
            worldPos, normal, viewDir, lightDir, light.color,
            light.intensity, attenuation, albedo, metallic, roughness, F0, shadow
        );
    }

    // Process each spotlight
    for (int i = 0; i < numSpotLights; ++i) {
        SpotLight light = spotLights[i];
        vec3 lightVec = light.position - worldPos;
        float distance = length(lightVec);
        vec3 lightDir = normalize(lightVec);

        // Skip if outside of range
        if (distance > light.range) continue;

        // Calculate spotlight cone effect
        float theta = dot(lightDir, normalize(-light.direction));

        // Skip if outside the outer cone
        if (theta < light.outerCutoff) continue;

        // Calculate spotlight intensity with smooth transition between inner and outer cone
        float epsilon = light.innerCutoff - light.outerCutoff;
        float spotIntensity = clamp((theta - light.outerCutoff) / epsilon, 0.0, 1.0);

        // Calculate distance-based attenuation
        float attenuation = max(0.0, 1.0 - distance / light.range);
        attenuation = attenuation * attenuation;

        // Combine spot intensity with attenuation
        float finalAttenuation = attenuation * spotIntensity;

        // No shadow calculation for spotlight for now
        float shadow = calculateSpotShadow(worldPos, light);

        // Calculate and add spotlight contribution using the unified function
        lighting += calculatePBRLighting(
            worldPos, normal, viewDir, lightDir, light.color,
            light.intensity, finalAttenuation, albedo, metallic, roughness, F0, shadow
        );
    }

    // More conservative tone mapping - preserve darks better
    // Use ACES-inspired filmic tone mapping that preserves deep shadows
    vec3 x = max(vec3(0.0), lighting - 0.004);
    vec3 mapped = (x * (6.2 * x + 0.5)) / (x * (6.2 * x + 1.7) + 0.06);

    // Gamma correction
    // mapped = pow(mapped, vec3(1.0/2.2));

    // Final color output
    FragColor = vec4(mapped, 1.0);
}

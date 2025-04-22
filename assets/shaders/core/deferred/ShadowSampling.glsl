float PCFSampling(vec3 projCoords, sampler2D shadowMap, int kernelSize, float bias) {
    float shadowFactor = 0.0;
    vec2 texelSize = 1.0 / vec2(textureSize(shadowMap, 0));

    // Get depth from shadow map once to avoid redundant texture fetches
    float currentDepth = projCoords.z;

    // Use hardware PCF if available (gives better performance)
    for (int x = -1; x <= 1; ++x) {
        for (int y = -1; y <= 1; ++y) {
            vec2 offset = vec2(float(x), float(y)) * texelSize;
            float depthSample = texture(shadowMap, projCoords.xy + offset).r;
            shadowFactor += (currentDepth - bias > depthSample) ? 1.0 : 0.0;
        }
    }

    float numSamples = float(kernelSize * kernelSize);
    shadowFactor /= numSamples;
    return shadowFactor;
}

float sampleCascadeAtlas(vec3 projCoords, int shadowMapIndex, int cascadeIndex, int numCascades, float bias) {
    // For horizontal strip layout, each cascade uses the full height but 1/numCascades of the width
    float stripWidth = 1.0 / float(numCascades);

    // Add a small inset to avoid sampling across cascade boundaries
    float inset = 0.001;

    // Scale coordinates to fit within the strip and avoid edge bleeding
    float adjustedX = projCoords.x * (stripWidth - 2.0 * inset) + inset;
    float adjustedY = projCoords.y;

    // Calculate the final UV coordinates in the atlas
    // For horizontal strips, x is offset by cascadeIndex * stripWidth
    vec2 atlasCoords = vec2(
        adjustedX + cascadeIndex * stripWidth,
        adjustedY
    );

    // Get shadow map dimensions
    vec2 texelSize = 1.0 / vec2(textureSize(shadowMaps[shadowMapIndex], 0));

    // PCF Sampling with 3x3 kernel
    float shadowFactor = 0.0;
    int kernelSize = 3;

    for (int x = -1; x <= 1; ++x) {
        for (int y = -1; y <= 1; ++y) {
            // Limit offset to prevent sampling across strip boundaries
            vec2 offset = vec2(
                clamp(float(x) * texelSize.x, -inset, inset),
                float(y) * texelSize.y
            );

            float depthSample = texture(shadowMaps[shadowMapIndex], atlasCoords + offset).r;
            shadowFactor += (projCoords.z - bias > depthSample) ? 1.0 : 0.0;
        }
    }

    shadowFactor /= 9.0; // 3x3 kernel = 9 samples
    return shadowFactor;
}

// Updated cascade calculation function to work with slice depths from DirectionalLight
float calculateCascadedShadow(vec3 fragPos, DirectionalLight light) {
    if (light.castShadow == 0) return 0.0; // No shadow if shadow casting is disabled

    // Get the normal from the G-buffer
    vec3 normal = normalize(texture(gNormal, TexCoords).rgb);
    vec3 lightDir = normalize(-light.dir);

    // Calculate bias based on surface angle to light
    float NdotL = max(dot(normal, lightDir), 0.0);
    float baseBias = 0.0005;
    float angleFactor = 1.0 - NdotL;
    float bias = clamp(baseBias * (1.0 + angleFactor * 5.0), 0.0001, 0.005);

    // Get view space depth
    float fragViewDepth = (view * vec4(fragPos, 1.0)).z;

    // Find which cascade to use based on slice depths
    int cascadeIndex = light.numCascades - 1; // Default to last cascade
    for (int i = 0; i < light.numCascades - 1; i++) {
        if (fragViewDepth > light.cascadeSliceDepths[i]) {
            cascadeIndex = i;
            break;
        }
    }

    // Get the light space matrix for this cascade
    mat4 lightMatrix = lightMatrices[light.lightMatrixIndex + cascadeIndex];

    // Transform fragment position to light space
    vec4 fragPosLightSpace = lightMatrix * vec4(fragPos, 1.0);
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;

    // Transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;

    // Check if fragment is in light frustum
    if (projCoords.z < 0.0 || projCoords.z > 1.0 ||
        projCoords.x < 0.0 || projCoords.x > 1.0 ||
        projCoords.y < 0.0 || projCoords.y > 1.0) {
        return 0.0;
    }

    // Sample the shadow map
    float shadowFactor = sampleCascadeAtlas(
        projCoords,
        light.shadowMapIndex,
        cascadeIndex,
        light.numCascades,
        bias
    );

    // Blend between cascades to avoid hard transitions
    if (cascadeIndex < light.numCascades - 1) {
        float nextSliceDepth = light.cascadeSliceDepths[cascadeIndex + 1];
        float blendZone = nextSliceDepth * 0.9;

        if (fragViewDepth < blendZone && fragViewDepth > nextSliceDepth) {
            // Calculate blend factor
            float blendFactor = smoothstep(
                nextSliceDepth,
                blendZone,
                fragViewDepth
            );

            // Only blend if needed
            if (blendFactor > 0.0) {
                // Sample next cascade
                int nextCascadeIndex = cascadeIndex + 1;
                mat4 nextLightMatrix = lightMatrices[light.lightMatrixIndex + nextCascadeIndex];

                vec4 nextFragPosLightSpace = nextLightMatrix * vec4(fragPos, 1.0);
                vec3 nextProjCoords = nextFragPosLightSpace.xyz / nextFragPosLightSpace.w;
                nextProjCoords = nextProjCoords * 0.5 + 0.5;

                if (nextProjCoords.z >= 0.0 && nextProjCoords.z <= 1.0 &&
                    nextProjCoords.x >= 0.0 && nextProjCoords.x <= 1.0 &&
                    nextProjCoords.y >= 0.0 && nextProjCoords.y <= 1.0) {

                    float nextShadowFactor = sampleCascadeAtlas(
                        nextProjCoords,
                        light.shadowMapIndex,
                        nextCascadeIndex,
                        light.numCascades,
                        bias
                    );

                    // Blend shadow factors
                    shadowFactor = mix(shadowFactor, nextShadowFactor, blendFactor);
                }
            }
        }
    }

    // For visualization of cascade levels (useful for debugging)
    // Uncomment to see which cascade is used for each pixel
    /*
    if (cascadeIndex == 0) return vec3(shadowFactor, 0.0, 0.0);
    else if (cascadeIndex == 1) return vec3(0.0, shadowFactor, 0.0);
    else if (cascadeIndex == 2) return vec3(0.0, 0.0, shadowFactor);
    else return vec3(shadowFactor, shadowFactor, 0.0);
    */

    return shadowFactor;
}

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

    // Check if frag is in light view frustum
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;
    if (projCoords.x < 0.0 || projCoords.x > 1.0 ||
        projCoords.y < 0.0 || projCoords.y > 1.0 ||
        projCoords.z < 0.0 || projCoords.z > 1.0) {
        return 0.0;
    }

    float shadowFactor = PCFSampling(projCoords, shadowMaps[light.shadowMapIndex], 3, bias);

    // This creates more natural shadow transitions
    float currentDepth = projCoords.z;
    float shadowDistance = abs(currentDepth - texture(shadowMaps[light.shadowMapIndex], projCoords.xy).r);
    float shadowEdgeFactor = smoothstep(0.0, bias * 2.0, shadowDistance);

    // Apply subtle shadow strength adjustment based on distance
    // Shadows become slightly weaker at distance for a more realistic look
    float shadowStrength = mix(1.0, 0.85, distanceFactor);

    return shadowFactor * shadowStrength;
}

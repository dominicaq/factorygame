#version 330 core
out vec4 FragColor;

in vec2 TexCoord;

// G-Buffer
uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedo;
uniform sampler2D gPBRParams;
uniform sampler2D gDepth;

// Debug mode
uniform int debugMode;

uniform float u_Near;
uniform float u_Far;

// Depth slicing
uniform int numSlices;

float LinearizeDepth(float depth) {
    float z = depth * 2.0 - 1.0; // Back to NDC
    return (2.0 * u_Near * u_Far) / (u_Far + u_Near - z * (u_Far - u_Near));
}

// Hash function to generate a random color based on the slice index
vec3 GetSliceColor(int slice) {
    float r = fract(sin(float(slice) * 12.9898) * 43758.5453);
    float g = fract(sin(float(slice) * 78.233) * 43758.5453);
    float b = fract(sin(float(slice) * 93.989) * 43758.5453);
    return vec3(r, g, b);
}

void main()
{
    if (debugMode == 0) {
        // Show position buffer
        FragColor = vec4(texture(gPosition, TexCoord).rgb, 1.0);
    } else if (debugMode == 1) {
        // Show normal buffer
        FragColor = vec4(texture(gNormal, TexCoord).rgb, 1.0);
    } else if (debugMode == 2) {
        // Show albedo buffer
        FragColor = texture(gAlbedo, TexCoord);
    } else if (debugMode == 3) {
        // Show metallic buffer - extract from the R channel of gPBRParams
        float metallic = texture(gPBRParams, TexCoord).r;
        FragColor = vec4(vec3(metallic), 1.0);
    } else if (debugMode == 4) {
        // Show roughness buffer - extract from the G channel of gPBRParams
        float roughness = texture(gPBRParams, TexCoord).g;
        FragColor = vec4(vec3(roughness), 1.0);
    } else if (debugMode == 5) {
        // Show ambient occlusion buffer - extract from the B channel of gPBRParams
        float ao = texture(gPBRParams, TexCoord).b;
        FragColor = vec4(vec3(ao), 1.0);
    } else if (debugMode == 6) {
        // Visualize the depth buffer directly
        float depth = texture(gDepth, TexCoord).r;
        float linearDepth = LinearizeDepth(depth);

        // Normalize the depth value between near and far planes
        float normalizedDepth = (linearDepth - u_Near) / (u_Far - u_Near);
        normalizedDepth = clamp(normalizedDepth, 0.0, 1.0);

        // Output as grayscale
        FragColor = vec4(vec3(normalizedDepth), 1.0);
    } else if (debugMode == 7) {
        // Depth slice visualization
        float depth = texture(gDepth, TexCoord).r;
        float linearDepth = LinearizeDepth(depth);

        // Calculate the slice index based on the provided formula
        float logFarNear = log(u_Far / u_Near);
        float sliceIndex = (log(linearDepth) / logFarNear) * numSlices - (numSlices * log(u_Near) / logFarNear);

        // Get the integer slice index and clamp it within the range
        int slice = int(floor(sliceIndex));
        slice = clamp(slice, 0, numSlices - 1);

        // Get a consistent random color for this slice
        vec3 sliceColor = GetSliceColor(slice);
        FragColor = vec4(sliceColor, 1.0);
    }
}

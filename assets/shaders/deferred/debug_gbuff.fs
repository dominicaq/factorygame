#version 330 core
out vec4 FragColor;

in vec2 TexCoord;

// G-Buffer
uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedo;
uniform sampler2D gDepth;

// Debug mode
uniform int debugMode;

uniform float u_Near;
uniform float u_Far;

float LinearizeDepth(float depth) {
    float z = depth * 2.0 - 1.0; // Back to NDC
    return (2.0 * u_Near * u_Far) / (u_Far + u_Near - z * (u_Far - u_Near));
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
        // Visualize the depth buffer directly
        float depth = texture(gDepth, TexCoord).r;

        // Linearize depth for better visualization
        float linearDepth = LinearizeDepth(depth);

        // Normalize the depth value between near and far planes
        float normalizedDepth = (linearDepth - u_Near) / (u_Far - u_Near);
        normalizedDepth = clamp(normalizedDepth, 0.0, 1.0);

        // Output as grayscale
        FragColor = vec4(vec3(normalizedDepth), 1.0);
    }
}

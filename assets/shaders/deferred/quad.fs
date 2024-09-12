#version 330 core
out vec4 FragColor;

in vec2 TexCoord;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedo;
uniform int debugMode;

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
    }
}

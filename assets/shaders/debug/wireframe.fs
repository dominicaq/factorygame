#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform vec4 u_AlbedoColor;
uniform float u_Time;

void main()
{
    float pulseIntensity = sin(u_Time * 5) * 0.25 + 0.75;

    // Apply pulsing effect to the albedo color
    vec3 pulsedColor = u_AlbedoColor.xyz * pulseIntensity;

    FragColor = vec4(pulsedColor, 1.0f);
}

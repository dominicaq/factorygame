#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform vec3 u_AlbedoColor;
uniform float u_Time;

void main()
{
    float pulseIntensity = sin(u_Time) * 0.5 + 0.5;

    // Apply pulsing effect to the albedo color
    vec3 pulsedColor = u_AlbedoColor * pulseIntensity;

    FragColor = vec4(pulsedColor, 1.0f);
}

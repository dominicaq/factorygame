#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform vec3 u_AlbedoColor;
uniform bool u_HasNormalMap;

void main()
{
    if (u_HasNormalMap) {
        FragColor = vec4(u_AlbedoColor * 0.5, 1.0f);
    } else {
        FragColor = vec4(u_AlbedoColor, 1.0f);
    }
}

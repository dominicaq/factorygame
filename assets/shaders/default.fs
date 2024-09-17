#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform vec3 u_AlbedoColor;

void main()
{
    FragColor = vec4(u_AlbedoColor, 1.0f);
}

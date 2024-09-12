#version 330 core
layout(location = 0) out vec3 FragPos;
layout(location = 1) out vec3 Normal;
layout(location = 2) out vec4 Albedo;

in vec3 WorldPos;
in vec3 NormalDir;
in vec2 TexCoords;

uniform sampler2D texture_diffuse1;

void main() {
    FragPos = WorldPos;
    Normal = normalize(NormalDir);
    Albedo = texture(texture_diffuse1, TexCoords);
}

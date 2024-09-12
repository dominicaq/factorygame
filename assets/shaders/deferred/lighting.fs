#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedo;

void main() {
    vec3 FragPos = texture(gPosition, TexCoords).rgb;
    vec3 Normal = normalize(texture(gNormal, TexCoords).rgb);
    vec4 Albedo = texture(gAlbedo, TexCoords);

    // Basic lighting (e.g., directional light)
    vec3 lightDir = normalize(vec3(0.0, -1.0, -1.0));
    float diff = max(dot(Normal, lightDir), 0.0);
    vec3 diffuse = diff * vec3(1.0, 1.0, 1.0); // white light

    vec3 result = diffuse * Albedo.rgb;
    FragColor = vec4(result, 1.0);
}

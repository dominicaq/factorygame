#version 330 core

in vec2 TexCoords;
in vec3 FragPos;
out vec4 FragColor;

// G-buffer textures
uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedo;

// Scene Data
uniform vec3 u_CameraPosition;
uniform vec3 u_LightPosition;
uniform vec3 u_LightColor;
uniform float u_LightIntensity;

void main() {
    // Retrieve data from G-buffer
    vec3 FragPos = texture(gPosition, TexCoords).rgb;
    vec3 Normal = normalize(texture(gNormal, TexCoords).rgb);
    vec3 Albedo = texture(gAlbedo, TexCoords).rgb;

    // Ambient component
    vec3 ambient = 0.2 * Albedo;

    // Diffuse lighting
    vec3 lightDir = normalize(u_LightPosition - FragPos);
    float diff = max(dot(Normal, lightDir), 0.0);
    vec3 diffuse = diff * u_LightColor * u_LightIntensity * 0.8;

    // Specular lighting
    vec3 viewDir = normalize(u_CameraPosition - FragPos);
    vec3 reflectDir = reflect(-lightDir, Normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 64.0);
    vec3 specular = u_LightColor * spec * u_LightIntensity * 0.5;

    // Combine lighting components
    vec3 lighting = ambient + diffuse + specular;

    // Blend albedo and lighting
    FragColor = vec4(mix(Albedo, lighting, 0.5), 1.0);
}

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

// Light structure
struct Light {
    vec3 position;
    vec3 color;
    float intensity;
    bool isDirectional;
    vec3 direction;
};

#define MAX_LIGHTS 10
uniform int numLights;
uniform Light lights[MAX_LIGHTS];

void main() {
    // Retrieve data from G-buffer
    vec3 FragPos = texture(gPosition, TexCoords).rgb;
    vec3 Normal = normalize(texture(gNormal, TexCoords).rgb);
    vec3 Albedo = texture(gAlbedo, TexCoords).rgb;

    // Ambient component
    vec3 ambient = 0.2 * Albedo;
    vec3 lighting = ambient;

    // Accumulate lighting for each light (naive approach)
    for (int i = 0; i < numLights; ++i) {
        Light light = lights[i];

        vec3 lightDir;
        if (light.isDirectional) {
            lightDir = normalize(-light.direction);
        } else {
            lightDir = normalize(light.position - FragPos);
        }

        // Diffuse component
        float diff = max(dot(Normal, lightDir), 0.0);
        vec3 diffuse = diff * light.color * light.intensity * 0.8;

        // Specular component
        vec3 viewDir = normalize(u_CameraPosition - FragPos);
        vec3 reflectDir = reflect(-lightDir, Normal);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), 512.0);
        vec3 specular = light.color * spec * light.intensity * 0.5;

        // Accumulate light contribution
        lighting += diffuse + specular;
    }

    // Blend albedo and lighting
    FragColor = vec4(mix(Albedo, lighting, 0.5), 1.0);
}

#version 330 core

in vec2 TexCoords;
out vec4 FragColor;

// G-buffer textures
uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedo;

// Camera position
uniform vec3 u_CameraPosition;

// Light struct matching the C++ struct
struct PointLight {
    vec4 position;
    vec4 color;
    vec4 intensity;
};

// SSBO binding point 0
layout(std430, binding = 0) buffer PointLightsBuffer {
    PointLight lights[];
};

uniform int numLights;

void main() {
    vec3 FragPos = texture(gPosition, TexCoords).rgb;
    vec3 Normal = normalize(texture(gNormal, TexCoords).rgb);
    vec3 Albedo = texture(gAlbedo, TexCoords).rgb;

    vec3 lighting = vec3(0.0);

    for (int i = 0; i < numLights; ++i) {
        PointLight light = lights[i];

        vec3 lightPos = light.position.xyz;
        vec3 lightColor = light.color.rgb;
        float intensity = light.intensity.x;

        vec3 lightDir = normalize(lightPos - FragPos);
        float distance = length(lightPos - FragPos);
        float attenuation = 1.0 / (distance * distance);

        // Diffuse shading
        float diff = max(dot(Normal, lightDir), 0.0);
        vec3 diffuse = diff * lightColor * intensity;

        // Specular shading
        vec3 viewDir = normalize(u_CameraPosition - FragPos);
        vec3 reflectDir = reflect(-lightDir, Normal);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), 64.0);
        vec3 specular = spec * lightColor * intensity;

        // Accumulate lighting
        lighting += (diffuse + specular) * attenuation;
    }

    // Combine with albedo
    FragColor = vec4(lighting * Albedo, 1.0);
}

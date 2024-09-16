#version 330 core

out vec4 FragColor;

in vec2 TexCoords;

// G-buffer textures
uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedo;

// Camera position for specular calculation
uniform vec3 u_CameraPosition;

// Light properties (for a single light)
uniform vec3 u_LightPosition;
uniform vec3 u_LightColor;
uniform float u_LightIntensity;
uniform bool u_IsDirectional;
uniform vec3 u_LightDirection;

void main()
{
    // Retrieve data from G-buffer
    vec3 FragPos = texture(gPosition, TexCoords).rgb;
    vec3 Normal = normalize(texture(gNormal, TexCoords).rgb);
    vec3 Albedo = texture(gAlbedo, TexCoords).rgb;

    // Ambient component (slightly reduced to emphasize lighting and texture)
    vec3 ambient = 0.2 * Albedo;

    // Diffuse lighting
    vec3 lightDir;
    if (u_IsDirectional) {
        // Directional light
        lightDir = normalize(-u_LightDirection);
    } else {
        // Point light (ignore attenuation for now)
        lightDir = normalize(u_LightPosition - FragPos);
    }

    // Diffuse calculation
    float diff = max(dot(Normal, lightDir), 0.0);
    vec3 diffuse = diff * u_LightColor * u_LightIntensity * 0.8;

    // Specular lighting
    vec3 viewDir = normalize(u_CameraPosition - FragPos);
    vec3 reflectDir = reflect(-lightDir, Normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 512.0);
    vec3 specular = u_LightColor * spec * u_LightIntensity * 0.5;

    // Final color (combine albedo and lighting)
    vec3 lighting = ambient + diffuse + specular;

    // Blend lighting and albedo to ensure texture visibility while showing lighting influence
    FragColor = vec4(mix(Albedo, lighting, 0.5), 1.0);
}

#version 330 core

// Inputs from vertex shader
in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

// Output color
out vec4 FragColor;

// Uniforms for lighting calculations
uniform vec3 u_LightPos = vec3(10.0, 10.0, 10.0); // Hardcoded light position
uniform vec3 u_LightColor = vec3(1.0, 1.0, 1.0); // White light
uniform vec3 u_ObjectColor = vec3(1.0, 0.5, 0.31); // Object's color (orange-ish)

// Light and view position
uniform vec3 u_ViewPos; // Position of the camera

void main() {
    // Normalize the input normal
    vec3 norm = normalize(Normal);

    // Calculate the vector from the fragment position to the light position
    vec3 lightDir = normalize(u_LightPos - FragPos);

    // Diffuse shading: Lambertian reflection
    float diff = max(dot(norm, lightDir), 0.0);

    // Specular shading
    float specularStrength = 0.5;
    vec3 viewDir = normalize(u_ViewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * u_LightColor;

    // Final color including diffuse and specular
    vec3 diffuse = diff * u_LightColor;
    vec3 result = (diffuse + specular) * u_ObjectColor;

    FragColor = vec4(result, 1.0);
}

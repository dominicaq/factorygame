#version 330 core

// Output color of the fragment
out vec4 FragColor;

// Input from vertex shader
in vec3 FragPos;   // Position of the fragment in world space
in vec3 Normal;    // Normal vector from vertex shader

void main() {
    // Hardcoded light and object colors
    vec3 objectColor = vec3(1.0, 0.5, 0.31);  // Object color (orange)

    // Output the final color
    FragColor = vec4(objectColor, 1.0);
}

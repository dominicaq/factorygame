#version 330 core

in vec3 Normal;
out vec4 FragColor;

void main() {
    // View normals
    FragColor = vec4(Normal * 0.5 + 0.5, 1.0);
}

#version 330 core

// Full-screen quad positions
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoords;

// Output to the fragment shader
out vec2 TexCoords;

void main()
{
    // Pass texture coordinates through to the fragment shader
    TexCoords = aTexCoords;

    // Set the position of the vertex in clip space
    gl_Position = vec4(aPos, 1.0);
}

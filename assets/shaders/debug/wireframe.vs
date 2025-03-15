#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 2) in vec2 aTexCoord;
layout(location = 1) in vec3 aNormal;

uniform mat4 u_Model;
uniform mat4 u_View;
uniform mat4 u_Projection;

out vec2 TexCoords;

void main()
{
    TexCoords = aTexCoord;
    gl_Position = u_Projection * u_View * u_Model * vec4(aPos, 1.0);
}

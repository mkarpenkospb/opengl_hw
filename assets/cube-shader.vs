#version 330 core
layout (location = 0) in vec3 aPos;

out vec3 TexCoords;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 u_mvp2;

void main()
{
    TexCoords = aPos;
    gl_Position = u_mvp2 * vec4(aPos, 1.0);
}
#version 330 core

layout (location = 0) in vec3 in_position;
layout (location = 1) in vec3 in_normal;
layout (location = 2) in vec2 in_texcoord;


out vec3 Normal;
out vec2 tex_coord;
out vec3 Position;


uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
//https://habr.com/ru/post/347750/
void main()
{
    Normal = normalize(mat3(transpose(inverse(model))) * in_normal);
    Position = vec3(model * vec4(in_position, 1.0)); // not screen position
    tex_coord = in_texcoord;
    gl_Position = projection * view * model * vec4(in_position.xyz, 1.0);
}

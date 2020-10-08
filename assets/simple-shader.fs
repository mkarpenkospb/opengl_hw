#version 330 core

out vec4 o_frag_color;
layout(origin_upper_left) in vec4 gl_FragCoord;
struct vx_output_t
{
    vec3 color;
};

in vx_output_t v_out;

in vec3 Normal;
in vec3 Position;

in vec2 tex_coord;

uniform sampler2D u_tex;
uniform vec3 cameraPos;
uniform samplerCube skybox;
//https://habr.com/ru/post/347750/
void main()
{

    vec3 texture = texture(u_tex, tex_coord).rgb;

    vec3 I = normalize(Position - cameraPos);
    vec3 R = reflect(I, normalize(Normal));
    vec3 Rt = refract(Position, normalize(Normal), 1.50);
    o_frag_color = vec4(0.5 * texture(skybox, R).rgb + 0.2 * texture(skybox, Rt).rgb + 0.3 * texture, 1.0);

}

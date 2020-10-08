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
//
    vec3 I = normalize(Position - cameraPos);
    vec3 R = reflect(I, normalize(Normal));
    o_frag_color = vec4(texture(skybox, R).rgb + 0.3 * texture, 1.0);
//    vec3 norm = normalize(Normal);
//    vec3 lightDir = normalize(vec3(-1, 1, -1) - gl_FragCoord.xyz);
//    float diff = max(dot(norm, lightDir), 0.0);
//    vec3 diffuse = diff * texture;
//
//    o_frag_color = vec4(texture, 1.0f);

}

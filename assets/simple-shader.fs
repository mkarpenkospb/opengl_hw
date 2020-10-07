#version 330 core

out vec4 o_frag_color;
layout(origin_upper_left) in vec4 gl_FragCoord;
struct vx_output_t
{
    vec3 color;
};

in vx_output_t v_out;

uniform vec3 u_color;
uniform float u_time;

// https://habr.com/ru/post/206516/
// f(z) = z^2 + 0.285 + 0.01i

in vec3 Normal;
in vec2 tex_coord;

uniform sampler2D u_tex;

void main()
{
//     o_frag_color = vec4(v_out.color.xy,0,1.0);
    vec3 texture = texture(u_tex, tex_coord).rgb;

    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(vec3(-1, -1, -1) - gl_FragCoord.xyz);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * texture;

//    vec3 result = (ambient + diffuse) * sphereColor;
//    o_frag_color = vec4(diff, 1.0f);
    o_frag_color = vec4(texture, 1.0f);
//    vec2 f_coord = vec2((gl_FragCoord.x / u_resolution.x) * (x_up_right - x_down_left) + x_down_left,
//                        (gl_FragCoord.y / u_resolution.y) * (y_down_left - y_up_right) + y_up_right) ;
//
//    float tmp = 0;
//    int step = 0;
//    for (; step < u_iteration; ++step) {
//        if (length(f_coord) > (r * rscale)) {
//            break;
//        }
//        f_coord = vec2(f_coord.x * f_coord.x - f_coord.y * f_coord.y, 2 * f_coord.x * f_coord.y) + c;
//    }
//
//    o_frag_color = vec4(texture(myColors, (3.0 * step / u_iteration)).rgb, 1);
}

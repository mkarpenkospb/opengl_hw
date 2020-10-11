#version 330 core

out vec4 o_frag_color;
//layout(origin_upper_left) in vec4 gl_FragCoord;
struct vx_output_t
{
    vec3 color;
};

in vx_output_t v_out;

in vec3 Normal;
in vec3 Position;

in vec2 tex_coord;

uniform vec3 cameraPos;
uniform sampler2D u_tex;
uniform samplerCube skybox;

uniform float n_from; // воздух
uniform float n_to; //
uniform float a;


void main()
{

    vec3 i = normalize(Position - cameraPos); // вектор, который отражаем
    vec3 r = reflect(i, Normal); // отражение
    vec3 t = refract(i, Normal, n_from / n_to);
    // Schlick’s approximation from the article, but we don't need it
//    float R0 = pow((n_from - n_to) / (n_from + n_to), 2);
//    float R1 = R0 + (1-R0) * pow(1 - cosi, 5);
//    float R2 = R0 + (1-R0) * pow(1 - cost, 5);
    float cosi = dot(-i, Normal); // = |n| * |i| * cos = 1 * 1 * cos = cos, с -i он хотя бы не должен стать отрицательным
    float sin2t = pow(n_from / n_to, 2) * (1 - pow(cosi, 2));
    float cost = pow(1 - sin2t, 0.5);

    float R1 = pow((n_from * cosi - n_to * cost) / (n_from * cosi + n_to * cost), 2);
    float R2 = pow((n_to * cosi - n_from * cost) / (n_to * cosi + n_from * cost), 2);
    float R = (R1 + R2) / 2;
    bool TIR = sin2t > 1;
    if (TIR) { R = 1;}
    float T = 1 - R;

    o_frag_color = vec4(mix((R * texture(skybox, r).rgb + T * texture(skybox, t).rgb), texture(u_tex, tex_coord).rgb, a) , 1.0);
}

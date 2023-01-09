$input v_color0, v_normal, v_fragpos, v_texcoord0, v_texcoord1

#include <bgfx_shader.sh>
#include <common.sh>

SAMPLER2D(texture0, 0);
SAMPLER2D(texture1, 1);

void main() {
    vec3 norm = normalize(v_normal);
    vec3 viewDir = normalize(camera_viewpos - v_fragpos);
    float dist = length(u_camera_viewpos.xyz - v_fragpos.xyz);

    vec3 result_light_color = texture2D(texture1, v_texcoord1).xyz;
    vec3 dlightresult = calc_d_light(norm, viewDir) + result_light_color;
    gl_FragColor = mix_fog(dist, texture2D(texture0, v_texcoord0) * vec4(dlightresult, 1.0));
}

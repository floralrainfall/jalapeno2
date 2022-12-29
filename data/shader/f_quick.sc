$input v_color0, v_normal, v_fragpos

#include <bgfx_shader.sh>
#include <common.sh>

void main() {    
    vec3 viewDir = normalize(camera_viewpos - v_fragpos);
    float dist = length(u_camera_viewpos.xyz - v_fragpos.xyz);
    vec3 norm = normalize(v_normal);

    vec3 dlightresult = calc_d_light(norm, viewDir);

    gl_FragColor = mix(mix_fog(dist, v_color0 * vec4(dlightresult, 1.0)),vec4(v_normal,1.0), 0.0);
}

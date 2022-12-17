$input v_color0, v_normal, v_fragpos

#include <bgfx_shader.sh>
#include <common.sh>

void main() {
    vec3 norm = normalize(v_normal);
    vec3 viewDir = normalize(camera_viewpos - v_fragpos);
    float dist = length(u_camera_viewpos.xyz - v_fragpos.xyz);

    vec3 dlightresult = calc_d_light(norm, viewDir);
    for(int i = 0; i < NR_POINT_LIGHTS; i++)
        if(u_lights[i].position.w == 1.0)
            dlightresult += calc_u_light(u_lights[i], norm, v_fragpos, viewDir);

    gl_FragColor = mix_fog(dist,vec4(dlightresult, 1.0) * vec4(u_color.xyz, 1.0) * v_color0);
}

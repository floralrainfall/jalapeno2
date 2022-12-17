$input v_color0, v_normal, v_fragpos

#include <bgfx_shader.sh>
#include <common.sh>

void main() {    
    float dist = length(u_camera_viewpos.xyz - v_fragpos.xyz);

    gl_FragColor = mix_fog(dist, v_color0);
}

$input a_position, a_normal, a_texcoord0, a_color0
$output v_color0, v_normal, v_fragpos, v_texcoord0

#include <bgfx_shader.sh>

uniform mat3 u_modelMatrix;

void main() {
    gl_Position = mul(u_modelViewProj, vec4(a_position, 1.0));
    v_color0 = a_color0;
    // v_normal = mul(u_modelMatrix, a_normal);
    v_normal = a_normal;
    v_fragpos = mul(u_model[0], vec4(a_position, 1.0)).xyz;
    v_texcoord0 = a_texcoord0;
}

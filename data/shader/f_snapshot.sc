$input v_color0, v_normal, v_fragpos

#include <bgfx_shader.sh>
#include <common.sh>

void main() {
    vec3 norm = normalize(v_normal);

    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * worldlight_ambient;

    float diffuseStrength = 0.5;
    vec3 lightDirection = normalize(-worldlight_direction);
    float diff = max(dot(norm, lightDirection), 0.0);
    vec3 diffuse = diffuseStrength * diff * worldlight_diffuse;

    float specularStrength = 1.0;
    vec3 viewDir = normalize(camera_viewpos - v_fragpos);
    vec3 reflectDir = reflect(-lightDirection, norm);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * worldlight_specular;  

    gl_FragColor = vec4((specular + diffuse + ambient), 1.0) * v_color0;
}

uniform vec4 u_worldlight_direction;
uniform vec4 u_worldlight_ambient;
uniform vec4 u_worldlight_diffuse;
uniform vec4 u_worldlight_specular;
#define worldlight_direction u_worldlight_direction.xyz
#define worldlight_ambient u_worldlight_ambient.xyz
#define worldlight_diffuse u_worldlight_diffuse.xyz
#define worldlight_specular u_worldlight_specular.xyz

uniform vec4 u_camera_viewpos;
#define camera_viewpos u_camera_viewpos.xyz

uniform vec4 u_fog_color;
uniform vec4 u_fog_settings;
uniform vec4 u_material_settings;
uniform vec4 u_time;

struct Light 
{
    vec4 position;
    float constant;
    float linear;
    float quadratic;
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
};

#define NR_POINT_LIGHTS 4  
uniform Light u_lights[NR_POINT_LIGHTS];
uniform vec4 u_color;

vec4 mix_fog(float dist, vec4 color)
{
    float fog_maxdist = u_fog_settings.y;
    float fog_mindist = u_fog_settings.x;
    float fog_factor = (fog_maxdist - dist) /
                  (fog_maxdist - fog_mindist);
    fog_factor = clamp(fog_factor, 0.0, 1.0);
    return mix(u_fog_color, color, fog_factor);
}

vec3 calc_d_light(vec3 norm, vec3 viewDir)
{
    float ambientStrength = u_worldlight_ambient.w;
    vec3 ambient = ambientStrength * worldlight_ambient;

    float diffuseStrength = u_worldlight_diffuse.w;
    vec3 lightDirection = normalize(-worldlight_direction);
    float diff = max(dot(norm, lightDirection), 0.0);
    vec3 diffuse = diffuseStrength * diff * worldlight_diffuse;

    float specularStrength = u_worldlight_specular.w;
    vec3 reflectDir = reflect(-lightDirection, norm);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * worldlight_specular;  

    return (ambient + diffuse + specular);
}

vec3 calc_u_light(Light i, vec3 normal, vec3 fragpos, vec3 viewdir) {
    vec3 lightDir = normalize(i.position.xyz - fragpos);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewdir, reflectDir), 0.0), u_material_settings.x);
    float distance = length(i.position.xyz - v_fragpos);
    float attenuation = 1.0 / (i.constant + i.linear * distance + i.quadratic * (distance * distance));
    vec3 ambient = i.ambient.xyz;
    vec3 diffuse = i.diffuse.xyz * diff;
    vec3 specular = i.specular.xyz * spec;
    ambient  *= attenuation;
    diffuse  *= attenuation;
    specular *= attenuation;
    return (ambient + diffuse + specular);
}
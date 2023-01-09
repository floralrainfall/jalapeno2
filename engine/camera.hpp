#ifndef ENGINE_CAMERA_HPP
#define ENGINE_CAMERA_HPP
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <bgfx/bgfx.h>

enum CameraMode
{
    CM_TARGET,
    CM_FREE,
    CM_TARGETLH,
    CM_FREELH,
};

enum CameraProjection
{
    CP_MANUAL,
    CP_PERSPECTIVE,
    CP_ORTHOGRAPHIC,
    CP_PERSPECTIVELH,
    CP_ORTHOGRAPHICLH,
};

struct Camera
{
    CameraMode mode;
    CameraProjection projection;

    glm::mat4 proj;
    glm::vec3 position;
    glm::vec3 target;
    glm::vec3 up;
    glm::vec3 front;
    glm::vec3 right;
    glm::vec3 worldup = glm::vec3(0.0f, 1.0f, 0.0f);
    bgfx::UniformHandle u_camera_position;
    bgfx::UniformHandle u_fog_settings;
    bgfx::UniformHandle u_fog_color;
    bgfx::UniformHandle u_time;

    float fog_mindist;
    float fog_maxdist;
    glm::vec4 fog_color;

    float pitch;
    float yaw;

    float fov;

    struct
    {
        float near;
        float far;
        float fov;
    } perspective_settings;

    struct
    {
        float top;
        float bottom;
        float left;
        float right;
    } orthographic_settings;

    void Init();
    Camera();
    glm::mat4 GetViewMatrix();
    void Update();
};

#endif
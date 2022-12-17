#include "camera.hpp"

Camera::Camera()
{
    position = glm::vec3();
    target = glm::vec3();
    worldup = glm::vec3(0.f,1.f,0.f);
    front = glm::vec3(0.f,0.f,-1.0f);
    yaw = 0;
    pitch = 0;
    fog_maxdist = 32.f;
    fog_mindist = 0.1f;
    fog_color = glm::vec4(0.5f,0.5f,0.5f,1.f);
}

void Camera::Init()
{
    u_fog_settings = bgfx::createUniform("u_fog_settings", bgfx::UniformType::Vec4);
    u_fog_color = bgfx::createUniform("u_fog_color", bgfx::UniformType::Vec4);
}

glm::mat4 Camera::GetViewMatrix()
{
    switch(mode)
    {
        default:
        case CM_TARGET:
            return glm::lookAt(position, target, up);
        case CM_FREE:
            return glm::lookAt(position, position + front, up);
    }
}

void Camera::Update()
{
    bgfx::setUniform(u_fog_color, &fog_color);
    bgfx::setUniform(u_fog_settings, &fog_mindist);
    switch(mode)
    {
        default:
        case CM_TARGET:

            break;
        case CM_FREE:
            glm::vec3 front;
            front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
            front.y = sin(glm::radians(pitch));
            front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
            this->front = glm::normalize(front);
            right = glm::normalize(glm::cross(this->front, worldup));
            up = glm::normalize(glm::cross(right, this->front));
            break;
    }
    bgfx::setUniform(u_camera_position, &position);
}
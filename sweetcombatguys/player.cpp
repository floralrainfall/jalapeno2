#include "player.hpp"
#include <engine.hpp>

SCG::Player::Player(SceneNode* parent) : Scene::MeshNode(parent)
{
    mesh = engine_app->mesh_manager->GetMesh("jalapeno.gltf"); // TODO: make playermodels
    shader = engine_app->shader_manager->GetShader("textured");
    acceleration = glm::vec3(0.f);
    map = nullptr;
    SetName("Player");
}

#define SPEED_RATE 0.05f
#define MOUSE_SPEED_RATE 0.05f
#define FALL_SPEED_LIMIT 1.f

void SCG::Player::Update()
{
    MeshNode::Update();

    float horizontal = engine_app->input_manager->GetInput("Horizontal");
    float vertical = engine_app->input_manager->GetInput("Vertical");
    float mouse_horizontal = engine_app->input_manager->GetInput("MouseHorizontal");
    float mouse_vertical = engine_app->input_manager->GetInput("MouseVertical");

    glm::vec3 movement_vector = glm::vec3(0.f);

    if(mouse_vertical != 0.f)
    {
        transform.eulerRot.z += MOUSE_SPEED_RATE * mouse_vertical * engine_app->delta_time;
        pendingChanges = true;
    }
    if(vertical != 0.f)
    {
        movement_vector.x = SPEED_RATE * std::cos(glm::radians(transform.eulerRot.z)) * vertical * engine_app->delta_time;
        movement_vector.z = SPEED_RATE * std::sin(glm::radians(transform.eulerRot.z)) * vertical * engine_app->delta_time;        
        pendingChanges = true;
    }
    if(horizontal != 0.f)
    {
        movement_vector.x = SPEED_RATE * std::cos(glm::radians(transform.eulerRot.z)+(M_PI_2f)) * horizontal * engine_app->delta_time;     
        movement_vector.z = SPEED_RATE * std::sin(glm::radians(transform.eulerRot.z)+(M_PI_2f)) * horizontal * engine_app->delta_time;   
        pendingChanges = true;
    }

    movement_vector += SPEED_RATE * acceleration * engine_app->delta_time;
    glm::vec3 new_vec = transform.pos + movement_vector;

    if(false)
    {
        glm::vec3 ground = new_vec;
        ground.y -= 0.2;
        JPBSP::TraceOutput ground_ray = map->bsp_file.TraceSphere((float*)&transform.pos,(float*)&ground, 0.1f);
        engine_app->Logf("%f %i", ground_ray.fraction, ground_ray.all_solid);
        if(!ground_ray.all_solid)
        {
        }
        else
        {
            acceleration.y -= 0.005f * 1.f * engine_app->delta_time;
        }
        transform.pos = new_vec;    
    }
    else
    {
        transform.pos = new_vec;    
        if(transform.pos.y > 0.f)
        {
            acceleration.y -= 0.005f * 1.f * engine_app->delta_time;
        }
        else
        {
            acceleration.y = 0.f;
            transform.pos.y = 0.f;
            if(engine_app->input_manager->GetInput("Jump") == 1.f)
            {
                acceleration.y += 1.f;
            }
        }
    }


    engine_app->camera.up = glm::vec3(0.f,1.f,0.f);
    engine_app->camera.mode = CM_FREE;
    engine_app->camera.position = transform.pos;
    engine_app->camera.position.y += 0.5f;
    engine_app->camera.yaw = transform.eulerRot.z;
    float new_pitch = engine_app->camera.pitch + MOUSE_SPEED_RATE * mouse_horizontal * engine_app->delta_time;
    if(new_pitch > 85.f)
        new_pitch = 85.f;
    if(new_pitch < -85.f)
        new_pitch = -85.f;
    engine_app->camera.pitch = new_pitch;
    engine_app->camera.Update();
}


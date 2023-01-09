#include "scene.hpp"
#include "engine.hpp"
#include <easy/profiler.h>
#include <random>
#include <string>
#include "imgui/imgui.h"
#include "stb_image.h"

bgfx::UniformHandle Scene::DirectionalLightNode::u_direction;
bgfx::UniformHandle Scene::DirectionalLightNode::u_ambient;
bgfx::UniformHandle Scene::DirectionalLightNode::u_diffuse;
bgfx::UniformHandle Scene::DirectionalLightNode::u_specular;
bgfx::UniformHandle Scene::SceneNode::u_color;

START_SAVE_ENTRY_DATA_2(Scene::SceneNode)
    SAVE_ENTRY_DATA(offset_of(&Scene::SceneNode::flags), sizeof(Scene::SceneNode::flags), 0)
END_SAVE_ENTRY_DATA

Scene::SceneNode::SceneNode(SceneNode* parent)
{
    EASY_FUNCTION();
    if(parent)
    {
        parent->AddChild(this);
    }
    else
    {
        this->parent = nullptr;
    }

    transform.pos = glm::vec3();
    transform.eulerRot = glm::vec3();
    transform.scale = glm::vec3(1.0);
    transform.modelMatrix = GetLocalModelMatrix();
    transform.color = glm::vec4(1.0);

    pendingChanges = true;
    flags = 0;
    tag = 0;

    memset(uuid,16+1,0);
    memset(name,64+1,0);

    SetName("SceneNode");

    std::random_device dev;
    std::mt19937 rng(dev());


    const char *v = "0123456789abcdefABCDEF.";
    std::uniform_int_distribution<int> dist(0, strlen(v)-1);

    std::string res;
    for (int i = 0; i < 32; i++) {
        res += v[dist(rng)];
        res += v[dist(rng)];
    }

    strncpy(this->uuid,res.c_str(),32);
}

void Scene::SceneNode::SetName(const char* name)
{
    strncpy(this->name,name,64);
}

glm::mat4 Scene::SceneNode::GetLocalModelMatrix()
{
    EASY_FUNCTION();
    const glm::mat4 transformX = glm::rotate(glm::mat4(1.0f),
                         glm::radians(transform.eulerRot.x),
                         glm::vec3(1.0f, 0.0f, 0.0f));
    const glm::mat4 transformY = glm::rotate(glm::mat4(1.0f),
                         glm::radians(transform.eulerRot.y),
                         glm::vec3(0.0f, 1.0f, 0.0f));
    const glm::mat4 transformZ = glm::rotate(glm::mat4(1.0f),
                         glm::radians(transform.eulerRot.z),
                         glm::vec3(0.0f, 0.0f, 1.0f));

    // Y * X * Z
    const glm::mat4 rotationMatrix = transformY * transformX * transformZ;

    // translation * rotation * scale (also know as TRS matrix)
    return glm::translate(glm::mat4(1.0f), transform.pos) *
                rotationMatrix *
                glm::scale(glm::mat4(1.0f), transform.scale);
}

void Scene::SceneNode::Update()
{
    EASY_FUNCTION();
    if(pendingChanges)
    {
        if (parent)
            transform.modelMatrix = parent->transform.modelMatrix * GetLocalModelMatrix();
        else
            transform.modelMatrix = GetLocalModelMatrix();
        pendingChanges = false;
    }

    for (auto&& child : children)
    {
        child->Update();
    }
}

void Scene::SceneNode::RUpdate()
{
    EASY_FUNCTION();
    for (auto&& child : children)
    {
        child->RUpdate();
    }
}

void Scene::SceneNode::Render()
{
    EASY_FUNCTION();
    if((flags & NODETAG_INVISIBLE))
        return;
        
    for (auto&& child : children)
    {
        child->Render();
    }
}

void Scene::SceneNode::DbgWidgets()
{

}

Scene::SceneNode::FindResult Scene::SceneNode::GetChildClosestTo(uint64_t tag, glm::vec3 position, SceneNode* exclude)
{
    FindResult f;
    f.found = nullptr;
    f.distance = INFINITY;
    for (auto&& child : children)
    {
        if(child->tag == tag && child != exclude)
        {
            float dist = glm::length(child->transform.pos - position);
            f.distance = std::min(dist, f.distance);
            if(f.distance == dist)
                f.found = child;
        }
    }
    return f;
}

Scene::SkyboxNode::SkyboxNode(Scene::SceneNode* parent)
{

}

void Scene::SkyboxNode::LoadTexture(const char* texture)
{
    int width, height, nr_channels;
    char* data;
    bgfx::TextureHandle cubemap_texture = bgfx::createTextureCube(512*512*3,false,1,bgfx::TextureFormat::RGB8,0);
    char* texture_side_names[] = {
        "right",
        "left",
        "top",
        "bottom",
        "back",
        "front",
    };
    for(int i = 0; i < 6; i++)
    {
        char tex_name[128];
        snprintf(tex_name,128,"%s_%s",texture_side_names[i],texture);
        data = (char*)stbi_load(tex_name,&width,&height,&nr_channels,3);
        bgfx::updateTextureCube(cubemap_texture,1,i,0,0,0,width,height,bgfx::makeRef(data,width*height*3));    
    }
}

void Scene::SkyboxNode::Render()
{
    
}

void Scene::MeshNode::Render()
{
    EASY_FUNCTION();
    if((flags & NODETAG_INVISIBLE))
        return;

    glm::mat4 localMatrix = GetLocalModelMatrix();
    if(mesh)
    {
        for(int i = 0; i < mesh->parts->size(); i++)
        {
            Data::Part* meshPart = mesh->parts->at(i);
            bgfx::setVertexBuffer(0, meshPart->vbh);
            bgfx::setIndexBuffer(meshPart->ibh);
            bgfx::setTransform((float*)&localMatrix);
            if(shader)
            {
                meshPart->ApplyTextures(0);
                bgfx::setUniform(u_color, &transform.color);
                bgfx::submit(0, shader->program);
            }
            else
            {
                shader = engine_app->shader_manager->GetShader("simple"); // placeholder
            }
        }
    }

    SceneNode::Render();
}

Scene::DirectionalLightNode::DirectionalLightNode(SceneNode* parent) : SceneNode(parent)
{
    direction = glm::vec4();
    diffuse = glm::vec4();
    specular = glm::vec4();
    ambient = glm::vec4();
    SetName("Sunlight");
}

void Scene::DirectionalLightNode::RUpdate() 
{
    EASY_FUNCTION();
    bgfx::setUniform(u_direction, &direction);
    bgfx::setUniform(u_diffuse, &diffuse);
    bgfx::setUniform(u_specular, &specular);
    bgfx::setUniform(u_ambient, &ambient);

    SceneNode::RUpdate();
}

void Scene::DirectionalLightNode::Init()
{
    EASY_FUNCTION();
    Scene::SceneNode::u_color = bgfx::createUniform("u_color", bgfx::UniformType::Vec4);
    u_direction = bgfx::createUniform("u_worldlight_direction", bgfx::UniformType::Vec4);
    u_diffuse = bgfx::createUniform("u_worldlight_diffuse", bgfx::UniformType::Vec4);
    u_ambient = bgfx::createUniform("u_worldlight_ambient", bgfx::UniformType::Vec4);
    u_specular = bgfx::createUniform("u_worldlight_specular", bgfx::UniformType::Vec4);
    engine_app->Logf("DirectionalLightNode: created uniforms (%i,%i,%i,%i)", u_direction.idx, u_diffuse.idx, u_ambient.idx, u_specular.idx);
}

void Scene::DirectionalLightNode::DbgWidgets()
{
    ImGui::SliderFloat4("Direction", (float*)&direction, -1.f, 1.f);
    ImGui::SliderFloat4("Diffuse", (float*)&diffuse, 0.f, 1.f);
    ImGui::SliderFloat4("Specular", (float*)&specular, 0.f, 1.f);
    ImGui::SliderFloat4("Ambient", (float*)&ambient, 0.f, 1.f);
}
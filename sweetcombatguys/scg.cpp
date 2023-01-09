#include <engine.hpp>
#include <console.hpp>
#include <imgui/imgui.h>
#include "player.hpp"
#include <jpbsp.hpp>
#include "filesystem.hpp"

class SCGApp : public Engine::App
{
public:
    virtual bool NetworkEnabled() { return true; }
    virtual char* GetAppName() { return "Sweet Combat Guys"; }
    virtual char* GetAppDescription() { return "Be the Best at your Game"; }

    SCG::Player* local_player;
    Scene::DirectionalLightNode* d1;

    virtual void PreInit()
    {
        networking_enabled = false;
        focusMode = true;
    }

    virtual void ClientInit()
    {
        IO::FileSystem::AddDataPath({.path = "scg"});
        IO::FileSystem::AddDataPath({.path = "scg/quake"});
        PreLoad(Engine::LT_MESH, "teapot.obj");
        bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x0);
        Debug::Console::enabled = false;
        Debug::Console::accessible = true;

        camera.worldup = glm::vec3(0.f,1.f,0.f);
        camera.up = camera.worldup;
        camera.mode = CM_FREE;
        camera.projection = CP_PERSPECTIVE;
        camera.pitch = 0.f;
        camera.yaw = 0.f;
        
        Scene::MeshNode* m2 = new Scene::MeshNode(root_node);
        m2->mesh = mesh_manager->GetMesh("teapot.obj");
        m2->shader = shader_manager->GetShader("textured");
        m2->transform.pos.z = 5.0f;
        m2->pendingChanges = true;

        menu_bar_enabled = true;

        input_manager->AddInputFunction({.mode = Input::IM_KEYBOARD, .data = (void*)SDLK_BACKQUOTE},[&](){ focusMode = !focusMode; });
        input_manager->AddInput({.name = "Jump", .source = {.mode = Input::IM_KEYBOARD, .data = (void*)SDLK_SPACE}});

        JPBSP::BSPSceneNode* s1 = new JPBSP::BSPSceneNode("maps/test_bigbox.bsp", root_node);
        local_player->map = s1;
        local_player->transform.pos.y += 10.f;
        s1->draw_collision = true;
    }

    virtual void Init() {
        JPBSP::BSPFile::Init();

        local_player = new SCG::Player(root_node);
        Scene::MeshNode* m1 = new Scene::MeshNode(root_node);
        m1->mesh = mesh_manager->GetMesh("jalapeno.gltf");
        m1->shader = shader_manager->GetShader("textured");
        m1->transform.pos.z = 1.0f;
        m1->pendingChanges = true;
        d1 = new Scene::DirectionalLightNode(root_node);
        d1->ambient = glm::vec4(1.0f,0.9f,0.7f,1);
        d1->diffuse = glm::vec4(1.0f,0.9f,0.9f,1);
        d1->specular = glm::vec4(1.0f,1.0f,0.9f,1);
        d1->direction = glm::vec4(1.0f,-1.0f,1.0f,1);
    }

    virtual void Tick()
    {
        local_player->flags = NODETAG_INVISIBLE;
        if(netcontext)
        {
            if(netcontext->active)
            {
                root_node->flags = NODETAG_INVISIBLE;
            }
            else
            {
                root_node->flags = 0;
            }
        }
    }

    virtual void GUIRender() {
        ImDrawList* drawList = ImGui::GetBackgroundDrawList();
        ImGui::Begin("Stuff");

        ImGui::End();
    }
};

Engine::App* LoadApp()
{
    SCGApp* current_app = new SCGApp();
    return current_app;
}
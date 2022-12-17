#include <engine.hpp>
#include "map.hpp"
#include "building.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <imgui/imgui.h>

class EoAApp : public Engine::App
{
public:
    EoA::MapNode* map;

    virtual void Init() 
    {
        IO::FileSystem::AddDataPath({.path = "eoa"});

        PreLoad(Engine::LT_TEXTURE, "ui/ui_assets.png");
        PreLoad(Engine::LT_MESH, "mesh_construction.gltf");
        PreLoad(Engine::LT_MESH, "mesh_city.gltf");

        ImColor skyrgb = ImColor();
        skyrgb.Value.x = 0.5294f;
        skyrgb.Value.y = 0.8078f;
        skyrgb.Value.z = 0.9216f;
        skyrgb.Value.w = 1.f;

        bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x87ceebff);
        camera.proj = glm::perspective(90.f,1.f,0.1f,1000.0f);
        camera.up = camera.worldup;
        camera.mode = CM_TARGET;
        camera.position = glm::vec3(0.f,10.f,10.f);
        camera.target = glm::vec3();
        camera.fog_maxdist = MAP_WIDTH/2.f;
        camera.fog_mindist = 0.1f;
        camera.fog_color = glm::vec4(skyrgb.Value.x,skyrgb.Value.y,skyrgb.Value.z,1);

        Scene::DirectionalLightNode* d1 = new Scene::DirectionalLightNode(root_node);
        d1->ambient = glm::vec4(1.0f,0.9f,0.7f,1);
        d1->diffuse = glm::vec4(1.0f,0.9f,0.9f,1);
        d1->specular = glm::vec4(1.0f,1.0f,0.9f,1);
        d1->direction = glm::vec4(1.0f,-1.0f,1.0f,1);

        map = new EoA::MapNode(root_node);
        EoA::BuildingNode* b1 = new EoA::BuildingNode(map, root_node);
    }

    virtual void Tick() 
    {

    }

    virtual void GUIRender()
    {
        glm::mat4 view_matrix = camera.GetViewMatrix();
        ImGui::Begin("Map");
        ImDrawList* mapDrawList = ImGui::GetWindowDrawList();
        ImVec2 wpos = ImGui::GetWindowPos();
        //char txt[64];
        int tile_width = 16;
        int tile_height = 16;
        for(int i = 0; i < MAP_WIDTH; i++)
        {
            for(int j = 0; j < MAP_HEIGHT; j++)
            {
                EoA::MapTileData d = map->GetTileData(i,j);
                glm::vec3 dc = map->GetTileColor(d.tile, d.height * 10.f);
                ImVec2 b = ImVec2(32+wpos.x+(tile_width*i),32+wpos.y+(tile_height*j));
                ImVec2 e = ImVec2(32+wpos.x+(tile_width*i)+tile_width,32+wpos.y+(tile_height*j)+tile_height);
                ImColor c(dc.x, dc.y, dc.z);

                c.Value.x *= d.height;
                c.Value.y *= d.height;
                c.Value.z *= d.height;
                mapDrawList->AddRectFilled(b,e,c);
                //mapDrawList->AddRect(b,e,0xffffffff);
                //snprintf(txt,64,"%.02f",d.height);
                //mapDrawList->AddText(b,0xffffffff,txt);
            }
        }
        ImGui::End();
    }
};

Engine::App* LoadApp()
{
    EoAApp* current_app = new EoAApp();
    return current_app;
}
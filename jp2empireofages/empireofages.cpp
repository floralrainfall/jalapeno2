#include <engine.hpp>
#include "map.hpp"
#include "building.hpp"
#include "polity.hpp"
#include "eoa.hpp"
#include <imgui/imgui.h>
#include <console.hpp>
#include <ui.hpp>

Data::Texture *EoA::building_types_sheet = nullptr;
Data::Texture *EoA::gen_assets_sheet = nullptr;

class EoAApp : public Engine::App
{
public:
    EoA::MapNode* map;
    EoA::MapNode* wanted_map;
    EoA::Polity* player_polity;

    EoA::BuildingNode* selected_node;

    enum { MainMenu, Game, NewGameMenu, BoatMode } game_mode = MainMenu;

    virtual void Init() 
    {
        IO::FileSystem::AddDataPath({.path = "eoa"});
        Debug::Console::enabled = false;
        Debug::Console::accessible = true;

        PreLoad(Engine::LT_TEXTURE, "ui/ui_assets.png");
        PreLoad(Engine::LT_TEXTURE, "ui/ui_stingers.png");
        PreLoad(Engine::LT_TEXTURE, "ui/ui_btypes.png");

        PreLoad(Engine::LT_MESH, "mesh_construction.gltf");
        PreLoad(Engine::LT_MESH, "mesh_city.gltf");
        PreLoad(Engine::LT_MESH, "mesh_unit_guy.gltf");

        EoA::gen_assets_sheet = texture_manager->GetTexture("ui/ui_assets.png");
        EoA::gen_assets_sheet->sprite_width = 64;
        EoA::gen_assets_sheet->sprite_height = 64;

        EoA::building_types_sheet = texture_manager->GetTexture("ui/ui_btypes.png");
        EoA::building_types_sheet->sprite_width = 128;
        EoA::building_types_sheet->sprite_height = 64;

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
        d1->ambient = glm::vec4(0.173f,0.255f,0.398f,1);
        d1->diffuse = glm::vec4(1.0f,0.9f,0.9f,1);
        d1->specular = glm::vec4(1.0f,1.0f,0.9f,1);
        d1->direction = glm::vec4(1.0f,-1.0f,1.0f,1);
    }

    float camera_x = 0.f;
    float camera_y = 0.f;

    virtual void Tick() 
    {
        switch(game_mode)
        {
            case MainMenu:

                break;
            case Game:
                if(selected_node)
                {
                    camera.target = selected_node->transform.pos;
                    camera.position = selected_node->transform.pos;
                    camera.position.x += std::sin(glm::radians((float)frame_counter)) * 3.f;
                    camera.position.z += std::cos(glm::radians((float)frame_counter)) * 3.f;
                    camera.position.y += 5;

                    EoA::Polity::TickPolities();
                }
                break;
            case BoatMode:
                {
                    camera.yaw += input_manager->GetInput("Horizontal") * delta_time / 10.f;
                    float camera_new_x = camera_x, camera_new_y = camera_y;
                    camera_new_x += std::cos(glm::radians(camera.yaw)) * input_manager->GetInput("Vertical") * delta_time / 10.f;
                    camera_new_y += std::sin(glm::radians(camera.yaw)) * input_manager->GetInput("Vertical") * delta_time / 10.f;
                    EoA::MapTileData d = map->GetTileData((int)camera_new_x, (int)camera_new_y);

                    if(d.tile != EoA::MOUNTAIN)
                    {
                        camera_x = camera_new_x;
                        camera_y = camera_new_y;
                    }
                    camera.position = glm::vec3(-MAP_HEIGHT/2 + camera_x, 10.f + d.height * 100.f, -MAP_HEIGHT/2 + camera_y);
                }
                break;
            case NewGameMenu:
                camera.mode = CM_TARGET;
                camera.target = glm::vec3(0.f,16.f,0.f);
                camera.position = glm::vec3(std::sin(frame_counter/50.f)*(MAP_WIDTH/4.f),32.f,std::cos(frame_counter/50.f)*(MAP_HEIGHT/4.f));
                break;
        }
    }

    virtual void GUIRender()
    {
        glm::vec4 uv_pos;
        ImVec2 size = ImVec2(18,18);
        if(game_mode == Game)
        {
            menu_bar_enabled = false;
            if(ImGui::BeginMainMenuBar())
            {
                if(ImGui::BeginMenu("Game"))
                {
                    if(ImGui::MenuItem("Quit"))
                    {
                        running = false;
                    }

                    ImGui::EndMenu();
                }

                if(ImGui::BeginMenu("Build"))
                {
                    ImGui::EndMenu();
                }

                uv_pos = EoA::gen_assets_sheet->GetSpriteUVs((int)EoA::GAS_Money);                
                ImGui::Image((ImTextureID)EoA::gen_assets_sheet->texture.idx,size,ImVec2(uv_pos.x,uv_pos.y),ImVec2(uv_pos.z,uv_pos.w));
                ImGui::Text(": %i $", player_polity->moneys);
                uv_pos = EoA::gen_assets_sheet->GetSpriteUVs((int)EoA::GAS_Goods);                
                ImGui::Image((ImTextureID)EoA::gen_assets_sheet->texture.idx,size,ImVec2(uv_pos.x,uv_pos.y),ImVec2(uv_pos.z,uv_pos.w));
                ImGui::Text(": %i kg", player_polity->goods);
                uv_pos = EoA::gen_assets_sheet->GetSpriteUVs((int)EoA::GAS_RawResources);                
                ImGui::Image((ImTextureID)EoA::gen_assets_sheet->texture.idx,size,ImVec2(uv_pos.x,uv_pos.y),ImVec2(uv_pos.z,uv_pos.w));
                ImGui::Text(": %i kg", player_polity->resources);
                uv_pos = EoA::gen_assets_sheet->GetSpriteUVs((int)EoA::GAS_Temperature);                
                ImGui::Image((ImTextureID)EoA::gen_assets_sheet->texture.idx,size,ImVec2(uv_pos.x,uv_pos.y),ImVec2(uv_pos.z,uv_pos.w));
                ImGui::Text(": %c%.2f degrees centigrade", (map->world_temperature_diff < 0.f)?'-':'+', map->world_temperature_diff);
                uv_pos = EoA::gen_assets_sheet->GetSpriteUVs((int)EoA::GAS_Co2);                
                ImGui::Image((ImTextureID)EoA::gen_assets_sheet->texture.idx,size,ImVec2(uv_pos.x,uv_pos.y),ImVec2(uv_pos.z,uv_pos.w));
                ImGui::Text(": %.2f ppm", map->world_co2_ppm);
                uv_pos = EoA::gen_assets_sheet->GetSpriteUVs((int)EoA::GAS_Rainfall + (int)map->weather);                
                ImGui::Image((ImTextureID)EoA::gen_assets_sheet->texture.idx,size,ImVec2(uv_pos.x,uv_pos.y),ImVec2(uv_pos.z,uv_pos.w));
            }
            ImGui::EndMainMenuBar();
            ImGui::Begin("Polities");
            EoA::Polity::ImGuiDrawPolities(player_polity);
            ImGui::End();
            if(selected_node)
            {
                ImGui::Begin("Building");
                if(selected_node->building_currently)
                {                  
                    uv_pos = EoA::gen_assets_sheet->GetSpriteUVs((int)EoA::GAS_Constructioning);                
                    ImGui::Image((ImTextureID)EoA::gen_assets_sheet->texture.idx,ImVec2(64,64),ImVec2(uv_pos.x,uv_pos.y),ImVec2(uv_pos.z,uv_pos.w));
                    ImGui::SameLine();  
                    ImGui::Text("Construction is %0.1f percent complete.", ((float)selected_node->building_progress / selected_node->building_next) * 100.f);
                    ImGui::Separator();
                }
                uv_pos = EoA::building_types_sheet->GetSpriteUVs((int)selected_node->type);
                ImGui::Image((ImTextureID)EoA::building_types_sheet->texture.idx,ImVec2(128,64),ImVec2(uv_pos.x,uv_pos.y),ImVec2(uv_pos.z,uv_pos.w));
                ImGui::Separator(); 
                uv_pos = EoA::gen_assets_sheet->GetSpriteUVs((int)EoA::GAS_Move);
                ImGui::ImageButton((ImTextureID)EoA::gen_assets_sheet->texture.idx,ImVec2(32,32),ImVec2(uv_pos.x,uv_pos.y),ImVec2(uv_pos.z,uv_pos.w));   
                uv_pos = EoA::gen_assets_sheet->GetSpriteUVs((int)EoA::GAS_Disassemble);
                ImGui::SameLine();
                ImGui::ImageButton((ImTextureID)EoA::gen_assets_sheet->texture.idx,ImVec2(32,32),ImVec2(uv_pos.x,uv_pos.y),ImVec2(uv_pos.z,uv_pos.w));   
                uv_pos = EoA::gen_assets_sheet->GetSpriteUVs((int)EoA::GAS_Destroy);
                ImGui::SameLine();
                ImGui::ImageButton((ImTextureID)EoA::gen_assets_sheet->texture.idx,ImVec2(32,32),ImVec2(uv_pos.x,uv_pos.y),ImVec2(uv_pos.z,uv_pos.w));    
                ImGui::Separator(); 
                if(ImGui::Button("Deselect"))
                {
                    selected_node = nullptr;
                }
                ImGui::End();
            }
            ImGui::Begin("Map");
            map->ImGuiDrawMap();
            ImGui::End();
        }
        else
        {
            menu_bar_enabled = true;
            switch(game_mode)
            {
                case BoatMode:
                    ImGui::Begin("Boat Mode");
                    ImGui::Text("Position: (%i,%i)", -MAP_HEIGHT/2 + (int)camera_x, -MAP_WIDTH/2 + (int)camera_y);
                    if(ImGui::Button("Go back to Main Menu"))
                    {
                        game_mode = MainMenu;
                        map->parent->children.remove(map);
                        delete map;
                    }
                    ImGui::End();
                    break;
                case MainMenu:            
                    ImGui::Begin("EMPIRE of AGES");
                    if(ImGui::Button("New Game"))
                    {
                        game_mode = NewGameMenu;
                        wanted_map = new EoA::MapNode(root_node);
                        wanted_map->seed = std::rand() % 1000;
                        wanted_map->GenerateMapData();
                        wanted_map->GenerateMap();
                        UI::im_aboutmenu_draw = false;

                        EoA::Polity::ClearPolities();
                        for(int i = 0; i < 4; i++)
                        {
                            EoA::Polity* new_polity = new EoA::Polity(wanted_map);
                            new_polity->human = false;
                            if(i == 0)
                                player_polity = new_polity;
                        }

                        player_polity->human = true;
                    }
                    if(ImGui::Button("Quit"))
                        running = false;
                    ImGui::End();
                    break;
                case NewGameMenu:
                    ImGui::Begin("New Game");    
                    wanted_map->ImGuiDrawMap();

                    if(ImGui::Button("Randomize Seed"))
                    {
                        wanted_map->seed = rand() % 10000;
                        wanted_map->GenerateMapData();
                        wanted_map->GenerateMap();
                    }

                    wanted_map->DbgWidgets();

                    ImGui::Separator();
                    EoA::Polity::ImGuiDrawPolities();
                    ImGui::Separator();
                    
                    if(ImGui::Button("Start Game"))
                    {
                        map = wanted_map;
                        wanted_map = 0;
                        game_mode = Game;
                    }
                    ImGui::SameLine();
                    if(ImGui::Button("Go Sailing"))
                    {
                        map = wanted_map;
                        wanted_map = 0;
                        game_mode = BoatMode;

                        camera.position = glm::vec3(0.f,0.f,0.f);
                        camera.mode = CM_FREE;
                    }
                    if(ImGui::Button("Back"))
                    {
                        wanted_map->parent->children.remove(wanted_map);
                        delete wanted_map;
                        game_mode = MainMenu;
                        EoA::Polity::ClearPolities();
                    }
                    ImGui::End();
                    break;
            }
        }
    }
};

Engine::App* LoadApp()
{
    EoAApp* current_app = new EoAApp();
    return current_app;
}
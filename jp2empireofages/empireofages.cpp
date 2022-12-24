#include <engine.hpp>
#include "map.hpp"
#include "building.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <imgui/imgui.h>
#include <console.hpp>

class EoAApp : public Engine::App
{
public:
    EoA::MapNode* map;
    EoA::MapNode* bm_map_s;
    EoA::MapNode* bm_map_n;
    EoA::MapNode* bm_map_w;
    EoA::MapNode* bm_map_e;
    EoA::MapNode* wanted_map;

    EoA::BuildingNode* selected_node;

    Data::Texture *building_types_sheet;
    Data::Texture *gen_assets_sheet;
    enum GenAssetSheetIds
    {
        GAS_DeadRelation,
        GAS_WarRelation,
        GAS_MadRelation,
        GAS_ShockedRelation,
        GAS_NeutralRelation,
        GAS_HappyRelation,
        GAS_AllianceRelation,
        GAS_SelfRelation,
        GAS_Money,
        GAS_Goods,
        GAS_Population,
        GAS_Happiness,
        GAS_Research,
        GAS_Food,
        GAS_Peace,
        GAS_War,
        GAS_Trade,
        GAS_Surrender,
        GAS_ScorchEarth,
        GAS_AdminMana,
        GAS_TradeMana,
        GAS_GoodsMoney,
        GAS_WorkMoney,
        GAS_Yes,
        GAS_No,
        GAS_UnknownOutcome,
        GAS_Construction,
        GAS_Intelligence,
        GAS_Asterix,
        GAS_Unknown,
        GAS_Defense = GAS_Unknown+4,
        GAS_Agility,
        GAS_Attack,
        GAS_Shock,
        GAS_Sneak,
        GAS_Ideas,
        GAS_GoodIncrease,
        GAS_BadDecrease,
        GAS_BadIncrease,
        GAS_GoodDecrease,
        GAS_Cohesion,
        GAS_Range,
        GAS_FirstPlace,
        GAS_SecondPlace,
        GAS_ThirdPlace,
        GAS_Gift,
        GAS_Logo,
        GAS_Disassemble,
        GAS_Move,
        GAS_Destroy,
        GAS_Constructioning
    };

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

        gen_assets_sheet = texture_manager->GetTexture("ui/ui_assets.png");
        gen_assets_sheet->sprite_width = 64;
        gen_assets_sheet->sprite_height = 64;

        building_types_sheet = texture_manager->GetTexture("ui/ui_btypes.png");
        building_types_sheet->sprite_width = 128;
        building_types_sheet->sprite_height = 64;

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
                camera.target = glm::vec3(0.f,16.f,0.f);
                camera.position = glm::vec3(std::sin(frame_counter/50.f)*(MAP_WIDTH/4.f),32.f,std::cos(frame_counter/50.f)*(MAP_HEIGHT/4.f));
                break;
        }
    }

    virtual void GUIRender()
    {
        if(game_mode == Game)
        {
            if(selected_node)
            {
                ImGui::Begin("Building");
                glm::vec4 uv_pos;
                if(selected_node->building_currently)
                {                  
                    uv_pos = gen_assets_sheet->GetSpriteUVs((int)GAS_Constructioning);                
                    ImGui::Image((ImTextureID)gen_assets_sheet->texture.idx,ImVec2(64,64),ImVec2(uv_pos.x,uv_pos.y),ImVec2(uv_pos.z,uv_pos.w));
                    ImGui::SameLine();  
                    ImGui::Text("Construction is %0.1f percent complete.", ((float)selected_node->building_progress / selected_node->building_next) * 100.f);
                    ImGui::Separator();
                }
                uv_pos = building_types_sheet->GetSpriteUVs((int)selected_node->type);
                ImGui::Image((ImTextureID)building_types_sheet->texture.idx,ImVec2(128,64),ImVec2(uv_pos.x,uv_pos.y),ImVec2(uv_pos.z,uv_pos.w));
                ImGui::Separator(); 
                uv_pos = gen_assets_sheet->GetSpriteUVs((int)GAS_Move);
                ImGui::ImageButton((ImTextureID)gen_assets_sheet->texture.idx,ImVec2(32,32),ImVec2(uv_pos.x,uv_pos.y),ImVec2(uv_pos.z,uv_pos.w));   
                uv_pos = gen_assets_sheet->GetSpriteUVs((int)GAS_Disassemble);
                ImGui::SameLine();
                ImGui::ImageButton((ImTextureID)gen_assets_sheet->texture.idx,ImVec2(32,32),ImVec2(uv_pos.x,uv_pos.y),ImVec2(uv_pos.z,uv_pos.w));   
                uv_pos = gen_assets_sheet->GetSpriteUVs((int)GAS_Destroy);
                ImGui::SameLine();
                ImGui::ImageButton((ImTextureID)gen_assets_sheet->texture.idx,ImVec2(32,32),ImVec2(uv_pos.x,uv_pos.y),ImVec2(uv_pos.z,uv_pos.w));    
                ImGui::Separator(); 
                ImGui::End();
            }
        }
        else
        {
            switch(game_mode)
            {
                case BoatMode:
                    ImGui::Begin("Boat Mode");
                    ImGui::Text("Position: (%i,%i)", -MAP_HEIGHT/2 + (int)camera_x, -MAP_WIDTH/2 + (int)camera_y);
                    if(ImGui::Button("Go back to Main Menu"))
                    {
                        game_mode = MainMenu;
                        wanted_map->parent->children.remove(map);
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
                    }
                    if(ImGui::Button("Quit"))
                        running = false;
                    ImGui::End();
                    break;
                case NewGameMenu:
                    ImGui::Begin("New Game");    
                    int tile_width = 1;
                    int tile_height = 1;        
                    ImGui::BeginChild("new game child frame", ImVec2(MAP_WIDTH*tile_width,MAP_HEIGHT*tile_height));
                    ImDrawList* mapDrawList = ImGui::GetWindowDrawList();
                    ImVec2 wpos = ImGui::GetWindowPos();
                    //char txt[64];
                    for(int i = 0; i < MAP_WIDTH; i++)
                    {
                        for(int j = 0; j < MAP_HEIGHT; j++)
                        {
                            EoA::MapTileData d = wanted_map->GetTileData(i,j);
                            glm::vec3 dc = wanted_map->GetTileColor(d.tile, d.height * 10.f);
                            ImVec2 b = ImVec2(wpos.x+(tile_width*i),wpos.y+(tile_height*j));
                            ImVec2 e = ImVec2(wpos.x+(tile_width*i)+tile_width,wpos.y+(tile_height*j)+tile_height);
                            ImColor c(dc.x/255.f, dc.y/255.f, dc.z/255.f);

                            //c.Value.x *= d.height;
                            //c.Value.y *= d.height;
                            //c.Value.z *= d.height;
                            mapDrawList->AddRectFilled(b,e,c);
                            //mapDrawList->AddRect(b,e,0xffffffff);
                            //snprintf(txt,64,"%.02f",d.height);
                            //mapDrawList->AddText(b,0xffffffff,txt);
                        }
                    }
                    ImGui::EndChild();

                    wanted_map->DbgWidgets();
                    
                    if(ImGui::Button("Start Game"))
                    {
                        map = wanted_map;
                        wanted_map = 0;
                        game_mode = Game;

                        glm::vec2 city_start;

                        EoA::MapTileData d;
                        while(d.tile != EoA::WATER && d.tile != EoA::MOUNTAIN)
                        {
                            glm::vec2 guess = glm::vec2(rand() % MAP_WIDTH, rand() % MAP_HEIGHT);
                            d = map->GetTileData(guess.x, guess.y);

                            if(d.tile != EoA::WATER && d.tile != EoA::MOUNTAIN && d.tile != EoA::UNKNOWN)
                            {
                                city_start = guess;
                            }
                        }

                        EoA::BuildingNode* city_node = new EoA::BuildingNode(map, root_node);
                        city_node->map_x = city_start.x;
                        city_node->map_y = city_start.y;
                        city_node->next_type = EoA::BT_City;
                        city_node->UpdateBuildingStage(true);
                        selected_node = city_node;
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
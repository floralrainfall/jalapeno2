#include "building.hpp"
#include "engine.hpp"
#include <imgui/imgui.h>

EoA::BuildingNode::BuildingNode(MapNode* map, Scene::SceneNode* parent) :
    Scene::MeshNode(parent)
{
    this->map = map;

    building_progress = 0;
    building_next = 100;
    building_currently = true;

    type = BT_ConstructionSite;
    next_type = BT_City;

    mesh = engine_app->mesh_manager->GetMesh("mesh_construction.gltf");
    shader = engine_app->shader_manager->GetShader("textured");

    map_x = MAP_WIDTH/2;
    map_y = MAP_HEIGHT/2;

    state = BS_Normal;

    UpdateBuildingStage();
}

void EoA::BuildingNode::Update()
{
    MapTileData d = map->GetTileData(map_x, map_y);
    transform.pos.x = -MAP_WIDTH/2 + map_x;
    transform.pos.y = d.height * 100.f + 1.f;
    transform.pos.z = -MAP_HEIGHT/2 + map_y;

    UpdateBuildingStage();
}

void EoA::BuildingNode::DbgWidgets()
{
    ImGui::InputInt("Building Type", (int*)&type);
    ImGui::InputInt("Building Next Type", (int*)&next_type);
    ImGui::Checkbox("Currently Building", &building_currently);
    ImGui::BeginDisabled(!building_currently);
    ImGui::SliderInt("Building Progress", &building_progress, 0, building_next);
    ImGui::EndDisabled();
    ImGui::InputInt("Building Next Ticks", &building_next);
    ImGui::InputInt2("Tile Position", &map_x);
}

void EoA::BuildingNode::UpdateBuildingStage(bool force)
{
    if(state == BS_Demolish)
        return;

    if(!force)
    {
        if(building_progress != building_next)
        {
            if(building_currently)
            {
                building_progress++;
                state = BS_Upgrading;
            }
            return;
        }
    }

    if(state == BS_Destroying)
    {
        state = BS_Demolish;
        return;
    }

    switch(next_type)
    {
        case BT_City:
            mesh = engine_app->mesh_manager->GetMesh("mesh_city.gltf");
            building_currently = false;
            type = next_type;
            next_type = BT_Unknown;
            break;
        case BT_Unknown:
            building_currently = false;
            return;
        default:
            building_currently = false;
            type = next_type;
            next_type = BT_Unknown;
            break;
    }
    building_progress = 0;
    state = BS_Normal;
}
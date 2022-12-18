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

    UpdateBuildingStage();
}

void EoA::BuildingNode::Update()
{
    MapTileData d = map->GetTileData(map_x, map_y);
    transform.pos.x = -MAP_WIDTH/2 + map_x;
    transform.pos.y = d.height * 10.f + 1.f;
    transform.pos.z = -MAP_HEIGHT/2 + map_y;

    UpdateBuildingStage();
}

void EoA::BuildingNode::UpdateBuildingStage()
{
    if(building_progress != building_next)
    {
        if(building_currently)
        {
            building_next++;
        }
        return;
    }

    switch(next_type)
    {
        case BT_City:
            mesh = engine_app->mesh_manager->GetMesh("mesh_city.gltf");
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
    type = next_type;
    building_progress = 0;
}
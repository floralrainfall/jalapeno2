#ifndef BUILDING_HPP
#define BUILDING_HPP

#include "map.hpp"

namespace EoA
{
    enum BuildingType
    {
        BT_City,
        BT_Barracks,
        BT_Farm,
        BT_ConstructionSite,
        BT_Unknown
    };

    class BuildingNode : public Scene::MeshNode
    {
        BuildingType type;
        BuildingType next_type;
        int building_progress;
        int building_next;
        bool building_currently;

        void UpdateBuildingStage();
    public:
        MapNode* map;

        BuildingNode(MapNode* map, Scene::SceneNode* parent);

        virtual void Update();

        int map_x;
        int map_y;
    };
};

#endif
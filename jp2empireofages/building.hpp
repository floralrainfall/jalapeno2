#ifndef BUILDING_HPP
#define BUILDING_HPP

#include "map.hpp"
#include "polity.hpp"

namespace EoA
{
    enum BuildingType
    {
        BT_City,
        BT_Barracks,
        BT_Farm,
        BT_ConstructionSite,
        BT_Unknown,
        BT_ResearchLab,
        BT_Factory,
        BT_CaravanDepot,
        BT_Fort,
        BT_IndustrialPark,
        BT_FBaitBroadcast,
        BT_Hub,
    };

    enum BuildingState
    {
        BS_Normal,
        BS_Destroying,
        BS_Demolish,
        BS_Upgrading,
    };

    class BuildingNode : public Scene::MeshNode
    {
    public:
        int building_progress;
        int building_next;
        EoA::Polity* player_owner;
        bool building_currently;
        BuildingType type;
        BuildingType next_type;
        BuildingState state;
        MapNode* map;

        BuildingNode(MapNode* map, Scene::SceneNode* parent);
        void UpdateBuildingStage(bool force = false);

        virtual void Update();
        virtual void DbgWidgets();

        int map_x;
        int map_y;
    };
};

#endif
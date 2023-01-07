#ifndef POLITY_HPP
#define POLITY_HPP

#include <glm/glm.hpp>
#include <list>
#include "map.hpp"

#define MAX_POLITIES 10

namespace EoA
{
    enum PolityStyle
    {
        DIPLOMAT,
        WARMONGER,
        TRADER,
        INDUSTRIALIST,
        EVIL,
        CLIMATIST,
        MILITARIST,

        Count,

        PLAYER
    };

    class Polity
    {
        static Polity* selected_polity;
        int wait_ticks;
        void* current_building; // lazy way to get out of doing some nonsense with includes, this is a EoA::BuildingNode*
        std::vector<void*> buildings; // yet another lazy way to get out of doing some nonsense with includes, this is a EoA::BuildingNode*
        static std::list<Polity*> polities;
    public:
        bool alive;
        glm::vec4 color;
        int number;
        bool human;

        int moneys; // in $
        int goods; // in kg
        int resources; // in kg
        int population; 
        int gas_emission; // in kg
        int military_size; // in units
        PolityStyle style;

        EoA::MapNode* map;

        Polity(EoA::MapNode* map);

        void AIThink();
        int CalcRelations(Polity* other_polity);
        static Polity* GetPolityByNumber(int id);
        static void TickPolities();
        static void ClearPolities();
        static void ImGuiDrawPolities(Polity* player = nullptr);
        void BuildBuilding(int type, int pos_x, int pos_y);
        static void SpawnHubs();
    };
}

#endif
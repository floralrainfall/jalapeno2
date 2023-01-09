#include "polity.hpp"
#include <random>
#include <imgui/imgui.h>
#include "eoa.hpp"
#include "building.hpp"

std::list<EoA::Polity*> EoA::Polity::polities;
EoA::Polity* EoA::Polity::selected_polity = 0;

static std::random_device rd;
static std::mt19937 gen(rd());
static std::uniform_real_distribution<> color_dist(0.5f, 1.f);
static std::uniform_int_distribution<> style_dist(0, EoA::PolityStyle::Count - 1);
static std::uniform_int_distribution<> x_position_dist(0, MAP_WIDTH);
static std::uniform_int_distribution<> y_position_dist(0, MAP_HEIGHT);

EoA::Polity::Polity(EoA::MapNode* map)
{
    number = polities.size();
    polities.push_back(this);

    color.x = color_dist(gen);
    color.y = color_dist(gen);
    color.z = color_dist(gen);

    alive = true;

    moneys = 2000;
    goods = 200;
    resources = 200;
    population = 1000;
    gas_emission = 0;
    military_size = 0;

    style = (PolityStyle)style_dist(gen);

    this->map = map;
}

void EoA::Polity::AIThink()
{
    // if the AI senses that their resources are lower then these,
    // it will do everything in its power to prop them back up

    int ai_lower_limit_goods = 100;
    int ai_lower_limit_resources = 75;
    int ai_lower_limit_moneys = 500;
    int ai_lower_limit_military = 15;

    BuildingNode* actual_building_node = (BuildingNode*)current_building;

    if(actual_building_node)
    {
        if(actual_building_node->building_currently == false)
        {   
            current_building = nullptr;
        }
        return;
    }

    if(wait_ticks)
    {
        wait_ticks--;
        return;        
    }

    if(ai_lower_limit_goods > goods)
    {

    }
    else if(ai_lower_limit_resources > resources)
    {

    }
    else if(ai_lower_limit_moneys > moneys)
    {

    }
}

int EoA::Polity::CalcRelations(Polity* other)
{
    int relation = 0;

    if(style == PLAYER)
        return 0;

    switch(style)
    {
        case WARMONGER: // these guys hate everything but wont go to war outright
            relation -= 10;
            relation -= other->gas_emission / 100;
            relation -= other->resources / 100;
            relation -= other->moneys / 1000 + other->goods / 100;
            relation -= other->military_size / 10;
            break;
        case DIPLOMAT: // love everyone slightly
            relation += 10;
            break;
        case CLIMATIST: // hate polluters
            relation -= other->gas_emission / 100;
            break;
        case INDUSTRIALIST: // love resources
            relation += other->resources / 100 + other->gas_emission / 1000;
            break;
        case TRADER: // love the money
            relation += other->moneys / 1000 + other->goods / 100;
            break;
        case EVIL: // just plain off evil
            relation = -1000;
            return relation;
        case MILITARIST:
            relation += other->military_size / 10;
            break;
    }

    return relation;
}

void EoA::Polity::TickPolities()
{
    for(auto&& polity : polities)
    {
        if(!polity->human)
            polity->AIThink();
    }
}

void EoA::Polity::ClearPolities()
{
    std::list<Polity*> refs_to_remove;
    for(auto&& polity : polities)
    {
        refs_to_remove.push_back(polity);
        delete polity;
    }
    for(auto&& polity : refs_to_remove)
    {
        polities.remove(polity);
    }
}

void EoA::Polity::ImGuiDrawPolities(Polity* player)
{
    glm::vec4 uv_pos;
    for(auto&& polity : polities)
    {
        uv_pos = gen_assets_sheet->GetSpriteUVs((int)(polity->human?GAS_HumanPlayer:GAS_AIPlayer));
        ImGui::Image((ImTextureID)gen_assets_sheet->texture.idx,ImVec2(18,18),ImVec2(uv_pos.x,uv_pos.y),ImVec2(uv_pos.z,uv_pos.w));
        ImGui::SameLine();
        if(player && polity != player)
        {
            int relationship_with_player = polity->CalcRelations(player);
            char polity_name[64];
            snprintf(polity_name, 64, "Player %i | %i",1 + polity->number, relationship_with_player);
            if(ImGui::Button(polity_name))
            {
                selected_polity = polity;
            }
        }
        else
            ImGui::TextColored(ImVec4(polity->color.x,polity->color.y,polity->color.z,1.f),"Player %i",1 + polity->number);
        if(!polity->human)
        {
            ImGui::SameLine();
            uv_pos = gen_assets_sheet->GetSpriteUVs((int)GAS_Diplomat+(int)polity->style);
            ImGui::Image((ImTextureID)gen_assets_sheet->texture.idx,ImVec2(18,18),ImVec2(uv_pos.x,uv_pos.y),ImVec2(uv_pos.z,uv_pos.w));
        }
    }

    if(selected_polity && player)
    {
        ImGui::Begin("Polity");
        if(selected_polity->style != PLAYER)
        {
            ImVec2 size = ImVec2(18,18);
            uv_pos = gen_assets_sheet->GetSpriteUVs((int)GAS_Diplomat+(int)selected_polity->style);
            ImGui::Image((ImTextureID)gen_assets_sheet->texture.idx,ImVec2(64,64),ImVec2(uv_pos.x,uv_pos.y),ImVec2(uv_pos.z,uv_pos.w));
            ImGui::SameLine();
            ImGui::Text("Style: %i\nRelationship to player: %i", (int)selected_polity->style, selected_polity->CalcRelations(player));
            ImGui::Separator();
            uv_pos = EoA::gen_assets_sheet->GetSpriteUVs((int)EoA::GAS_Money);                
            ImGui::Image((ImTextureID)EoA::gen_assets_sheet->texture.idx,size,ImVec2(uv_pos.x,uv_pos.y),ImVec2(uv_pos.z,uv_pos.w));            
            ImGui::SameLine();
            ImGui::Text(": %i $", selected_polity->moneys);
            uv_pos = EoA::gen_assets_sheet->GetSpriteUVs((int)EoA::GAS_Goods);                
            ImGui::Image((ImTextureID)EoA::gen_assets_sheet->texture.idx,size,ImVec2(uv_pos.x,uv_pos.y),ImVec2(uv_pos.z,uv_pos.w));
            ImGui::SameLine();
            ImGui::Text(": %i kg", selected_polity->goods);
            uv_pos = EoA::gen_assets_sheet->GetSpriteUVs((int)EoA::GAS_RawResources);                
            ImGui::Image((ImTextureID)EoA::gen_assets_sheet->texture.idx,size,ImVec2(uv_pos.x,uv_pos.y),ImVec2(uv_pos.z,uv_pos.w));     
            ImGui::SameLine();
            ImGui::Text(": %i kg", selected_polity->resources);
            uv_pos = EoA::gen_assets_sheet->GetSpriteUVs((int)EoA::GAS_Population);                
            ImGui::Image((ImTextureID)EoA::gen_assets_sheet->texture.idx,size,ImVec2(uv_pos.x,uv_pos.y),ImVec2(uv_pos.z,uv_pos.w));     
            ImGui::SameLine();
            ImGui::Text(": %i people", selected_polity->population);
            uv_pos = EoA::gen_assets_sheet->GetSpriteUVs((int)EoA::GAS_Intelligence);                
            ImGui::Image((ImTextureID)EoA::gen_assets_sheet->texture.idx,size,ImVec2(uv_pos.x,uv_pos.y),ImVec2(uv_pos.z,uv_pos.w));     
            ImGui::SameLine();
            ImGui::Text(": %i units", selected_polity->military_size);
        }
        ImGui::End();
    }
}

void EoA::Polity::BuildBuilding(int type, int x, int y)
{
    EoA::BuildingNode* city_node = new EoA::BuildingNode(map, map->parent);
    city_node->map_x = x;
    city_node->map_y = y;
    city_node->next_type = (EoA::BuildingType)type;
    city_node->player_owner = this;
    city_node->UpdateBuildingStage(true);
}

void EoA::Polity::SpawnHubs()
{
    for(auto&& polity : polities)
    {
        glm::vec2 city_start;

        EoA::MapTileData d;
        d.tile = EoA::WATER;
        while(d.tile != EoA::WATER && d.tile != EoA::MOUNTAIN)
        {
            glm::vec2 guess = glm::vec2(x_position_dist(gen), y_position_dist(gen));
            d = polity->map->GetTileData(guess.x, guess.y);

            if(d.tile != EoA::WATER && d.tile != EoA::MOUNTAIN && d.tile != EoA::UNKNOWN)
            {
                city_start = guess;
            }
        }

        polity->BuildBuilding((int)EoA::BT_Hub, city_start.x, city_start.y);
    }
}

EoA::Polity* EoA::Polity::GetPolityByNumber(int id)
{
    int c = 0;
    for(auto&& polity : polities)
    {
        if(c == id)
            return polity;
        c++;
    }
    return nullptr;
}
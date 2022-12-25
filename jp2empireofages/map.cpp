#include "map.hpp"
#include "mesh.hpp"
#include "engine.hpp"
#include "building.hpp"
#include "perlin.h"
#include <math.h>
#include <imgui/imgui.h>

EoA::MapNode::MapNode(Scene::SceneNode* parent) : Scene::SceneNode(parent)
{
    generated = false;
    for(int i = 0; i < MAP_WIDTH; i++)
    {
        for(int j = 0; j < MAP_HEIGHT; j++)
        {
            map[i][j].height = 0.f;
            map[i][j].tile = WATER;
        }
    }
    seed = 0;
    GenerateMapData();
    GenerateMap();
}

EoA::MapNode::~MapNode()
{    
    if(generated)
    {
        bgfx::destroy(vbh_map);
        bgfx::destroy(ibh_map);
    }
}

void EoA::MapNode::CalcVtxNormal(int a, int b, int c)
{
    Data::PosColorVertex& v_a = vertices.at(a);
    Data::PosColorVertex& v_b = vertices.at(b);
    Data::PosColorVertex& v_c = vertices.at(c);

    glm::vec3 normal = glm::cross(v_b.vertex-v_a.vertex, v_c.vertex-v_a.vertex);
    v_a.normal += normal;
    v_b.normal += normal;
    v_c.normal += normal;
}

void EoA::MapNode::GenerateMapData()
{
    world_ocean_level = ocean_threshold;
    weather = WS_Sunny;
    for(int i = 0; i < MAP_WIDTH; i++)
    {
        for(int j = 0; j < MAP_HEIGHT; j++)
        {
            MapTileData d;
            float fx = (float)i;
            float fy = (float)j;
            perlin_noise_seed = seed;
            d.height = perlin2d(fx/MAP_WIDTH,fy/MAP_HEIGHT,4,4)-0.5f;
            glm::vec2 center = glm::vec2(MAP_WIDTH/2,MAP_HEIGHT/2);
            glm::vec2 position = glm::vec2(fx,fy);
            if(glm::distance(center,position) < radial)
            {
                d.height *= std::cos(glm::distance(center,position)/radial * M_PI_2f) * radial_bias;
            }
            else
            {
                d.height = ocean_threshold;
            }
            d.tile = WATER;
            if(d.height <= ocean_threshold)
            {
                d.tile = WATER;
                d.height = ocean_threshold;
            }
            else if(d.height < beach_threshold)
            {
                d.tile = BEACH;
            }
            else if(d.height > mountain_threshold)
            {
                d.tile = MOUNTAIN;
            }
            else
            {
                perlin_noise_seed = seed + 1;
                float xtra_biome_sel = perlin2d(fx/32.f,fy/32.f,4,4);
                
                if(xtra_biome_sel < forest_threshold)
                {
                    d.tile = GRASS;
                }
                else
                {
                    d.tile = FOREST;
                }
            }
            map[i][j] = d;
        }
    }
}

EoA::MapTileData EoA::MapNode::GetTileData(int x, int y)
{
    if(x > 0 || x < MAP_WIDTH)
        if(y > 0 || y < MAP_HEIGHT)
        {
            if(world_ocean_level >= map[x][y].height)
                map[x][y].tile = WATER;
            return map[x][y];
        }
    MapTileData k = MapTileData();
    k.height = 0.f;
    k.tile = UNKNOWN;
    return k;
}

glm::vec3 EoA::MapNode::GetTileColor(MapTile t, float h)
{
    glm::vec3 v;
    bool heightfx = false;
    switch(t)
    {
        default:
            v.x = 255;
            v.y = 0;
            v.z = 255;
            break;
        case MapTile::WATER:
            v.x = 0;
            v.y = 0;
            v.z = 127;
            heightfx = true;
            h += 0.5f;
            break;
        case MapTile::GRASS:
            v.x = 0;
            v.y = 154,
            v.z = 23;
            heightfx = true;
            break;
        case MapTile::FOREST:
            v.x = 0;
            v.y = 54;
            v.z = 23;
            heightfx = true;
            break;
        case MapTile::BEACH:
            v.x = 255;
            v.y = 235;
            v.z = 205;
            break;
        case MapTile::MOUNTAIN:
            v.x = 64;
            v.y = 64;
            v.z = 64;
            heightfx = true;
            break;
    }
    if(heightfx)
    {
        v.x *= std::abs(h);
        v.y *= std::abs(h);
        v.z *= std::abs(h);
    }
    return v;
}

void EoA::MapNode::GenerateMap()
{
    if(generated)
    {
        bgfx::destroy(vbh_map);
        bgfx::destroy(ibh_map);

        vertices.clear();
        indices.clear();
    }
    else
        GenerateMapData();

    uint16_t vtx_id = 0;
    for(int i = 0; i < MAP_WIDTH; i++)
    {
        for(int j = 0; j < MAP_HEIGHT; j++)
        {
            Data::PosColorVertex v = {};

            MapTileData data = GetTileData(i,j);


            float fi = (float)i;
            float fj = (float)j;
            float x_pos = fi;
            float y_pos = fj;   

            float f_px = (x_pos)/MAP_WIDTH*2.f;
            float f_py = (y_pos)/MAP_HEIGHT*2.f;

            float height = data.height * 100.f;
            glm::vec3 colo = GetTileColor(data.tile, height / 10.f);

            v.r = colo.x;
            v.g = colo.y;
            v.b = colo.z;

            v.normal = glm::vec3(0.f, 1.f, 0.f);
            v.vertex = glm::vec3(-MAP_WIDTH/2.f + x_pos, std::max(world_ocean_level * 100.f, height), -MAP_HEIGHT/2.f + y_pos);

            vertices.push_back(v);

            vtx_id += 4;
        }
    }

    for(int i = 0; i < MAP_HEIGHT-1; i++)
    {
        for(int j = 0; j < MAP_WIDTH; j++)
        {
            for(int k = 0; k < 2; k++)
            {
                indices.push_back(j + MAP_WIDTH * (i + k));
            }
            CalcVtxNormal(j + MAP_WIDTH * (i), j + MAP_WIDTH * (i + 1), 0);
        }
    }

    // normalize normals
    for(int i = 0; i < vertices.size(); i++)
    {
        vertices[i].normal = glm::normalize(vertices[i].normal);
    }

    vbh_map = bgfx::createVertexBuffer(bgfx::makeRef(vertices.data(),sizeof(Data::PosColorVertex)*vertices.size()), Data::ms_layout);
    ibh_map = bgfx::createIndexBuffer(bgfx::makeRef(indices.data(),sizeof(uint16_t)*indices.size()));
    shader = engine_app->shader_manager->GetShader("quick");

    transform.color = glm::vec4(1.f);

    generated = true;
}

#define NUM_STRIPS (MAP_HEIGHT-1)
#define NUM_VERTS_PER_STRIP (MAP_WIDTH*2)

void EoA::MapNode::Render()
{
    if(generated)
    {
        for(int strip = 0; strip < NUM_STRIPS; ++strip)
        {
            glm::mat4 localMatrix = GetLocalModelMatrix();
            bgfx::setVertexBuffer(0, vbh_map);
            bgfx::setIndexBuffer(ibh_map, NUM_VERTS_PER_STRIP * strip, NUM_VERTS_PER_STRIP);
            bgfx::setTransform((float*)&localMatrix);
            bgfx::setState((BGFX_STATE_DEFAULT ^ BGFX_STATE_CULL_CW) | BGFX_STATE_PT_TRISTRIP | BGFX_STATE_CULL_CCW);
            bgfx::submit(0, shader->program);
        }
    }
}

void EoA::MapNode::DbgWidgets()
{
    SceneNode::DbgWidgets();
    if(ImGui::Button("Generate"))
    {
        GenerateMapData();
        GenerateMap();
    }
    ImGui::InputInt("Seed", &seed);
    ImGui::SliderFloat("Ocean Threshold", &ocean_threshold, -1.f, 1.f);
    ImGui::SliderFloat("Beach Threshold", &beach_threshold, -1.f, 1.f);
    ImGui::SliderFloat("Mountain Threshold", &mountain_threshold, -1.f, 1.f);
    ImGui::SliderFloat("Forest Threshold", &forest_threshold, -1.f, 1.f);
    ImGui::SliderFloat("Radial", &radial, -100.f, 100.f);
    ImGui::SliderFloat("Radial Bias", &radial_bias, -100.f, 100.f);
}

void EoA::MapNode::ImGuiDrawMap()
{
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
            EoA::MapTileData d = GetTileData(i,j);
            glm::vec3 dc = GetTileColor(d.tile, d.height * 10.f);
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
}
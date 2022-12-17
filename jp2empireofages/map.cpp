#include "map.hpp"
#include "mesh.hpp"
#include "engine.hpp"
#include "building.hpp"
#include "perlin.h"
#include <math.h>

EoA::MapNode::MapNode(Scene::SceneNode* parent) : Scene::SceneNode(parent)
{
    generated = false;
    GenerateMapData();
    GenerateMap();
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
    for(int i = 0; i < MAP_WIDTH; i++)
    {
        for(int j = 0; j < MAP_HEIGHT; j++)
        {
            MapTileData d;
            float fx = (float)i;
            float fy = (float)j;
            perlin_noise_seed = seed;
            d.height = perlin2d(fx/32.f,fy/32.f,4,4)-0.5f;
            d.tile = WATER;
            if(d.height < 0.f)
            {
                d.tile = WATER;
                d.height = 0.f;
            }
            else if(d.height < 0.1f)
            {
                d.tile = BEACH;
            }
            else if(d.height > 0.8f)
            {
                d.tile = MOUNTAIN;
            }
            else
            {
                perlin_noise_seed = seed + 1;
                float xtra_biome_sel = perlin2d(fx,fy,4,4);
                
                if(xtra_biome_sel < 0.5f)
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
    return map[x][y];
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

            float height = data.height * 10.f;

            v.normal = glm::vec3(0.f, 1.f, 0.f);
            v.vertex = glm::vec3(-MAP_WIDTH/2.f + x_pos, height, -MAP_HEIGHT/2.f + y_pos);

            glm::vec3 colo = GetTileColor(data.tile, height);

            v.r = colo.x;
            v.g = colo.y;
            v.b = colo.z;

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
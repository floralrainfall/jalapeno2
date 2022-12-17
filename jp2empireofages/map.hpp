#ifndef MAP_HPP
#define MAP_HPP

#include <scene.hpp>
#include <shader.hpp>
#include <bgfx/bgfx.h>

#define MAP_WIDTH 64
#define MAP_HEIGHT 64

namespace EoA
{
    enum MapTile
    {
        MOUNTAIN,
        GRASS,
        FOREST,
        WATER,
        BEACH,    
    };

    struct MapTileData
    {
        MapTile tile;
        float height;
    };

    class MapNode : public Scene::SceneNode
    {

        bool generated = false;
        std::vector<Data::PosColorVertex> vertices;
        std::vector<uint16_t> indices;
        void CalcVtxNormal(int a, int b, int c);
    public:
        int seed;
        MapTileData map[MAP_WIDTH][MAP_HEIGHT];

        bgfx::VertexBufferHandle vbh_map;
        bgfx::IndexBufferHandle ibh_map;
        Data::Shader* shader;

        void GenerateMap();
        void GenerateMapData();

        glm::vec3 GetTileColor(MapTile t, float h = 1.f);

        MapTileData GetTileData(int x, int y);

        virtual void Render();

        MapNode(Scene::SceneNode* parent);
    };
};

#endif
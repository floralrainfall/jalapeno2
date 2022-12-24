#ifndef MAP_HPP
#define MAP_HPP

#include <scene.hpp>
#include <shader.hpp>
#include <bgfx/bgfx.h>

#define MAP_WIDTH 192
#define MAP_HEIGHT 192

namespace EoA
{
    enum MapTile
    {
        MOUNTAIN,
        GRASS,
        FOREST,
        WATER,
        BEACH,    
        UNKNOWN,
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
        float ocean_threshold = 0.05f;
        float beach_threshold = 0.07f;
        float mountain_threshold = 0.150f;
        float forest_threshold = 0.34f;
        float radial = MAP_HEIGHT/2;
        float radial_bias = 1.f;
        MapTileData map[MAP_WIDTH][MAP_HEIGHT];

        bgfx::VertexBufferHandle vbh_map;
        bgfx::IndexBufferHandle ibh_map;
        Data::Shader* shader;

        void GenerateMap();
        void GenerateMapData();

        glm::vec3 GetTileColor(MapTile t, float h = 1.f);

        MapTileData GetTileData(int x, int y);

        virtual void Render();
        virtual void DbgWidgets();

        MapNode(Scene::SceneNode* parent);
        ~MapNode();
    };
};

#endif
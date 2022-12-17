#ifndef __JPBSP_HPP__
#define __JPBSP_HPP__

#include <glm/glm.hpp>
#include <bgfx/bgfx.h>
#include <vector>
#include <scene.hpp>

namespace JPBSP
{
    struct BSPEntry
    {
        int offset;
        int length;
    };

    struct BSPTexture
    {
        char name[64];
        int flags;
        int contents;
    };

    struct BSPLightmap
    {
        char data[128][128][3];
    };

    struct BSPPlane
    {
        float normal[3];
        float distance;
    };

    struct BSPNode
    {
        int plane;
        #define BSPNODE_FRONT 0
        #define BSPNODE_BACK  1
        int children[2];
        int mins[3];
        int maxs[3];
    };

    struct BSPLeaf
    {
        int cluster;
        int area;
        int mins[3];
        int maxs[3];
        int leafface;
        int n_leaffaces;
        int leafbrush;
        int n_leafbrushes;
    };

    struct BSPLeafFace
    {
        int face; // face idx
    };

    struct BSPLeafBrush
    {
        int brush; // brush idx
    };

    struct BSPModel
    {
        glm::vec3 mins;
        glm::vec3 maxs;
        int face;
        int n_faces;
        int brush;
        int n_brushes;
    };

    struct BSPBrush
    {
        int brushside;
        int n_brushsides;
        int texture;
    };

    struct BSPBrushSide
    {
        int plane;
        int texture;
    };

    // layout
    struct BSPVertex
    {
        float position[3];
        float texcoord[2];
        float lightcoord[2];
        float normal[3];
        char color[4];
    };

    struct BSPMeshVert
    {
        int offset;
    };

    struct BSPEffect
    {
        char name[64];
        int brush;
        int unknown;
    };

    struct BSPFace
    {
        uint32_t texture;
        uint32_t effect;
        uint32_t type;
        uint32_t vertex;
        uint32_t n_vertices;
        uint32_t meshvert;
        uint32_t n_meshverts;
        uint32_t lm_index;
        uint32_t lm_start[2];
        uint32_t lm_size[2];
        float lm_origin[3];    
        float lm_vecs[2][3];
        float normal[3];
        uint32_t size[2];
    };  

    struct BSPLoadedTexture
    {
        bgfx::TextureHandle texture;
        const char* name;
    };

    struct BSPMesh
    {
        bgfx::VertexBufferHandle vbh;
        bgfx::IndexBufferHandle ibh;     
        BSPLoadedTexture lightmap;
        BSPLoadedTexture texture;   
        Data::Shader* shader;
    };

    enum BSPEntryType
    {
        BET_ENTITIES,
        BET_TEXTURES,
        BET_PLANES,
        BET_NODES,
        BET_LEAFS,
        BET_LEAFFACES,
        BET_LEAFBRUSHES,
        BET_MODELS,
        BET_BRUSHES,
        BET_BRUSHSIDES,
        BET_VERTICES,
        BET_MESHVERTS,
        BET_EFFECTS,
        BET_FACES,
        BET_LIGHTMAPS,
        BET_LIGHTVOLS,
        BET_VISDATA,
        Count
    };

    struct BSPHeader
    {
        char magic[4]; // 'IBSP'
        int version; 

        BSPEntry entries[BSPEntryType::Count];
    };

    struct TraceOutput
    {
        float fraction;
        float end[3];
        bool starts_out;
        bool all_solid;

        enum {
            BSP_TRACETYPE_BOX,
            BSP_TRACETYPE_RAY,
            BSP_TRACETYPE_SPHERE
        } type;

        float radius;

        float mins[3];
        float maxs[3];
        float xtnts[3];
    };

    class BSPFile
    {

        BSPFace* bsp_faces;
        BSPVertex* bsp_vertices;
        BSPMeshVert* bsp_meshverts;

        void CheckBrush(BSPBrush* brush, float* start, float* end, TraceOutput* output);
        void CheckNode(int node_idx, float start_fraction, float end_fraction, float* start, float* end, TraceOutput* output);
        void ParseModel(BSPModel model);
        void ParseFace(BSPFace face);
        void Swizzle(float* v, bool s = false);
        void Swizzle(int* v);
        void FixVertex(BSPVertex& vertex);
        TraceOutput Trace(float* v_start, float* v_end, TraceOutput* output);
    public:
        void* bsp_data[BSPEntryType::Count];
        std::vector<BSPMesh> meshes;
        std::vector<BSPLoadedTexture> r_textures;
        std::vector<BSPLoadedTexture> r_lightmaps;
        BSPHeader header;
        bool loaded;

        static bgfx::VertexLayout bsp_layout;
        static bgfx::UniformHandle texture0;
        static bgfx::UniformHandle texture1;
        static void Init();
        BSPFile(const char* file);
        TraceOutput TraceBox(float* v_start, float* v_end, float* v_mins, float* v_maxs);
        TraceOutput TraceRay(float* v_start, float* v_end);
        TraceOutput TraceSphere(float* v_start, float* v_end, float radius);
        ~BSPFile();
    };

    class BSPSceneNode : public Scene::SceneNode
    {
        void RenderBrush(BSPBrush* brush);
        void RenderNode(int node_id);
        std::vector<BSPVertex> bbox_vertices;
        std::vector<uint16_t> bbox_indices;
        bgfx::VertexBufferHandle bbox_vbh;
        bgfx::IndexBufferHandle bbox_ibh;
        Data::Shader* bbox_shader;
    public:
        BSPFile bsp_file;

        BSPSceneNode(const char* file, SceneNode* parent);

        bool draw_collision = false;

        virtual void Render();
        virtual void DbgWidgets();
    };
};

#endif
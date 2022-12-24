#ifndef ENGINE_MESH_HPP
#define ENGINE_MESH_HPP

#include <bgfx/bgfx.h>
#include <vector>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <glm/glm.hpp>
#include "texture.hpp"
#include "shader.hpp"

namespace Data
{
    extern bgfx::VertexLayout ms_layout;

    struct MeshBoundingBox
    {
        float width; // x
        float width2; // -x
        float height; // y
        float height2; // -y
        float depth; // z
        float depth2; // -z
    };

    struct PosColorVertex
    {
        glm::vec3 vertex;
        glm::vec3 normal;
        glm::vec2 texcoord;
        glm::vec2 texcoord2;

        uint8_t r;
        uint8_t g;
        uint8_t b;
        uint8_t a;

        uint16_t bone_id[4];
        float bone_weight[4];
    };

    struct Part
    {
        bgfx::UniformHandle u_normalMatrix;
        glm::mat3 normalMatrix;

        bgfx::VertexBufferHandle vbh;
        bgfx::IndexBufferHandle ibh;

        std::vector<PosColorVertex> vertices;
        std::vector<uint16_t> indices;
        std::vector<Data::Texture*> textures;

        MeshBoundingBox bbox;

        void ApplyTextures(int stage);
    };

    struct Mesh
    {
        const char* file;
        std::vector<Part*> *parts;
        bgfx::TextureHandle snapshotHandle;

        MeshBoundingBox bbox;

        void ProcessNode(aiNode* node, const aiScene *scene);
        Part* ProcessMesh(aiMesh *mesh, const aiScene *scene);
        std::vector<Texture*> LoadMaterialTextures(aiMaterial *material, aiTextureType type, const char* name, const aiScene* scene);

        void RenderSnapshot();
    };

    class MeshManager
    {
    public:
        std::vector<Mesh*> meshes;
        MeshManager();
        void PrecacheLoadMesh(const char* path);
        bool RenderQueue();
        void RerenderAll(); // DEBUG
        Mesh* GetMesh(const char* path);
    };
}

#endif
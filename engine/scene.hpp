#ifndef ENGINE_SCENE_HPP
#define ENGINE_SCENE_HPP

#include <vector>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <list> 
#include <memory>
#include "mesh.hpp"
#include "shader.hpp"
#include "camera.hpp"
#include "save.hpp"
#include <bgfx/bgfx.h>

#define NODETAG_INVISIBLE (1<<1)
// do not replicate from server to client
#define NODETAG_NOREPLICATE (1<<2)
// do not replicate from client to server
#define NODETAG_NOSHADOW (1<<3)

namespace Scene
{
    struct Transform
    {
        glm::vec3 pos = { 0.0f, 0.0f, 0.0f };
        glm::vec3 eulerRot = { 0.0f, 0.0f, 0.0f };
        glm::vec3 scale = { 1.0f, 1.0f, 1.0f };    
        glm::mat4 modelMatrix = glm::mat4(1.0f);
        glm::vec4 color;
    };

    class SceneNode : public Save::ISaveable
    {
    public:
        CLASS_SAVEABLE;
        
        static bgfx::UniformHandle u_color;
        char uuid[32+1] = {0};
        char name[64+1] = {0};

        void SetName(const char* name);

        volatile bool pendingChanges = false;
        unsigned int flags;
        uint64_t tag;

        SceneNode* parent;
        std::list<SceneNode*> children;
        Transform transform;
        SceneNode(SceneNode* parent = nullptr);

        void AddChild(SceneNode* node)
        {
            children.emplace_back(node);
            children.back()->parent = this;
        }

        glm::mat4 GetLocalModelMatrix();
        virtual void Update();  // called on SERVER
        virtual void RUpdate(); // called on CLIENT
        virtual void Render();  // called on CLIENT
        virtual void DbgWidgets();
        // virtual void PushNetworkData();
        // virtual void PopNetworkData();

        struct FindResult
        {
            float distance;
            SceneNode* found;
        };
        FindResult GetChildClosestTo(uint64_t tag, glm::vec3 position, SceneNode* exclude = nullptr);
    };

    class SkyboxNode : public SceneNode
    {
    public:
        SkyboxNode(SceneNode* parent);
        void LoadTexture(const char* name);
        virtual void Render();
    };

    class MeshNode : public SceneNode
    {
    public:
        // CLASS_SAVEABLE;

        Data::Mesh* mesh;
        Data::Shader* shader;
        MeshNode(SceneNode* parent) : SceneNode(parent) { SetName("Mesh"); mesh = nullptr; shader = nullptr; };
        virtual void Render();
    };

    class DirectionalLightNode : public SceneNode
    {
    private:
        static bgfx::UniformHandle u_direction;
        static bgfx::UniformHandle u_ambient;
        static bgfx::UniformHandle u_diffuse;
        static bgfx::UniformHandle u_specular;
    public:
        // CLASS_SAVEABLE;

        glm::vec4 direction;
        glm::vec4 ambient;
        glm::vec4 diffuse;
        glm::vec4 specular;
        DirectionalLightNode(SceneNode* parent);
        virtual void DbgWidgets();
        virtual void RUpdate();
        static void Init();
    };

    class PointLightNode : public SceneNode 
    {
    public:
        // CLASS_SAVEABLE;
        
        float constant;
        float linear;
        float quadratic;
        glm::vec4 ambient;
        glm::vec4 diffuse;
        glm::vec4 specular;
    };
};
#endif
#include <engine.hpp>
#include <imgui/imgui.h>
#include <jpbsp.hpp>

class TestApp : public Engine::App
{
    public:
        ENGINE_VERSION_CHECK;
        
        virtual char* GetAppName() { return "Jalapeno 2 Test App"; }
        virtual char* GetAppDescription() { return "To test the jalapeno engine"; }

        Scene::DirectionalLightNode* d1;

        virtual void ClientInit()
        {
            PreLoad(Engine::LT_MESH, "data/teapot.obj");
        }

        virtual void Init()
        {
            JPBSP::BSPFile::Init();
            IO::FileSystem::AddDataPath({.path = "scg"});
            IO::FileSystem::AddDataPath({.path = "scg/quake"});
            Scene::MeshNode* m1 = new Scene::MeshNode(root_node);
            m1->mesh = mesh_manager->GetMesh("jalapeno.gltf");
            m1->shader = shader_manager->GetShader("textured");
            Scene::MeshNode* m2 = new Scene::MeshNode(root_node);
            m2->mesh = mesh_manager->GetMesh("teapot.obj");
            m2->transform.pos = glm::vec3(0.0f, 5.0f, 1.0f);
            m2->pendingChanges = true;
            d1 = new Scene::DirectionalLightNode(root_node);

            JPBSP::BSPSceneNode* s1 = new JPBSP::BSPSceneNode("maps/1smallmap.bsp", root_node);

            camera.proj = glm::perspective(90.f,1.f,0.1f,1000.0f);
            camera.up = camera.worldup;
            camera.target = glm::vec3(0,0,0);
            
            float time = ((float)SDL_GetTicks()) / 128.f;
            camera.position.x = std::sin(((float)time))*10;
            camera.position.z = std::cos(((float)time))*10;
            camera.position.y = std::sin(((float)time/10.f))*10;
            camera.position.x += std::sin(((float)time/10.f))/10;
            camera.position.z += std::cos(((float)time/20.f))/10;
            camera.position.y += std::sin(((float)time/30.f))/10;
        }

        virtual void Tick()
        {
        }

        virtual void GUIRender()
        {
            ImGui::Begin("Tests");
            ImGui::Text("Jalapeno2 %s", GetEngineVersion());
            if(ImGui::Button("Rerender Snapshots"))
            {
                mesh_manager->RerenderAll();
            }
            for(int i = 0; i < texture_manager->textures.size(); i++)
            {
                ImGui::Image((ImTextureID)texture_manager->textures[i]->texture.idx, ImVec2(128, 128));
                ImGui::Text("Image %p %s", texture_manager->textures[i]->texture.idx, texture_manager->textures[i]->name);
            }
            ImGui::Text("X, -Y, Z, ignore");
            ImGui::SliderFloat4("Light Direction", (float*)&d1->direction, -1.f, 1.f);
            ImGui::Text("R, G, B, M (oR = R * M)");
            ImGui::SliderFloat4("Light Ambient", (float*)&d1->ambient, 0.f, 1.f);
            ImGui::SliderFloat4("Light Specular", (float*)&d1->specular, 0.f, 1.f);
            ImGui::SliderFloat4("Light Diffuse", (float*)&d1->diffuse, 0.f, 1.f);
            d1->pendingChanges = true;
            ImGui::End();
        }
};

Engine::App* LoadApp()
{
    TestApp* current_app = new TestApp();
    return current_app;
}
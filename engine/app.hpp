#ifndef ENGINE_APP_HPP
#define ENGINE_APP_HPP
#include <SDL2/SDL.h>
#include "mesh.hpp"
#include "shader.hpp"
#include "scene.hpp"
#include "camera.hpp"
#include "texture.hpp"
#include "sound.hpp"
#include "input.hpp"
#include "gns/gns.hpp"
#include "filesystem.hpp"

#define ENGINE_VERSION_CHECK virtual const char* GetEngineVersion2() { return ENGINE_VERSION; };
#define ENGINE_VERSION "Pre-Alpha 0.1.0"

namespace Engine
{
    enum LoadType
    {
        LT_TEXTURE,
        LT_MESH,
        LT_SHADER,
    };
    class App
    {
        public:
            std::vector<IO::FileSystemDirectory> directories;

            volatile bool running = true;
            volatile double fps = 0.0;
            volatile float delta_time = 0.f;
            volatile uint64_t frame_counter = 0;

            bool pauseGame = false;
            bool focusMode = false;

            Network::SteamAppInfo steam_info;
            Camera camera;

            bool menu_bar_enabled = true;
            bool networking_enabled = false;
            Network::NetworkContext* netcontext = nullptr; // client network context
            Network::NetworkContext* netcontext_s = nullptr; // server network context

            Scene::SceneNode* root_node = nullptr;

            Data::MeshManager* mesh_manager = nullptr;
            Data::ShaderManager* shader_manager = nullptr;
            Data::TextureManager* texture_manager = nullptr;
            Sound::SoundManager* sound_manager = nullptr;
            Input::InputManager* input_manager = nullptr;

            virtual char* GetAppName();
            virtual char* GetAppDescription();
            static const char* GetEngineVersion() { return ENGINE_VERSION; };
            virtual const char* GetEngineVersion2() { return ENGINE_VERSION; }; // USE GetEngineVersion!! NOT THIS ONE!!

            // call in Init or PreInit
            void PreLoad(LoadType type, const char* file);
            void Logf(const char* format,...);

            virtual void Event(SDL_Event* event); // client
            virtual void PreInit(); // client/server before split
            virtual void NodeInitUniforms(); // client
            virtual void Init();       // initialize server side things
            virtual void ClientInit(); // initialize rendering/client side things
            virtual void PreRender(); // client
            virtual void PreTick(); // server
            virtual void Tick(); // server

            virtual void GUIRender();
    };
};

#endif
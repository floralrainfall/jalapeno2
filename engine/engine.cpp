#include "app.hpp"
#include "engine.hpp"
#include <bgfx/bgfx.h>
#include <bgfx/platform.h>
#include <bx/bx.h>
#include <bx/thread.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>
#include <future>
#include <easy/profiler.h>
#include "imgui/imgui.h"
#include "imgui/imgui_impl_bgfx.h"
#include "imgui/imgui_impl_sdl.h"
#include "console.hpp"
#include "ui.hpp"
#include "gns/gns.hpp"
#include "gns/network.hpp"
#include "filesystem.hpp"

Engine::App* engine_app = nullptr;

#define TICK_INTERVAL      16
static uint64_t next_time;
static uint64_t __fpslimit_time()
{
        uint64_t now = SDL_GetTicks();
        if(next_time <= now)
                return 0;
        else
                return next_time - now;
}
double lastFrame = 0.0f; // Time of last frame
SDL_Window* window;

#ifdef DEBUG

#endif

int main(int argc, char** argv)
{
    EASY_PROFILER_ENABLE;
    profiler::startListen();
    EASY_MAIN_THREAD;
    engine_app = LoadApp();
    // preinit
    SDL_Init(SDL_INIT_VIDEO);
    window = SDL_CreateWindow(engine_app->GetAppName(), 0, 0,
                            GAME_FIXED_WIDTH,
                            GAME_FIXED_HEIGHT,
                            SDL_WINDOW_SHOWN);
    srand(12461016);

    if(!engine_app)
    {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_WARNING, "LoadApp(); returned nullptr", ".", window);
    }
    else
    {
        if(strcmp(engine_app->GetEngineVersion2(),engine_app->GetEngineVersion())!=0)
        {
            SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Version mismatch", "engine_app and engine have different versions", window);
        }
        //SDL_GLContext context = SDL_GL_CreateContext(window);
        SDL_SysWMinfo wmi;
        SDL_VERSION(&wmi.version);
        if (!SDL_GetWindowWMInfo(window, &wmi)) {
            return false;
        }
        bgfx::PlatformData pd;
#if !BX_PLATFORM_EMSCRIPTEN
#if BX_PLATFORM_LINUX || BX_PLATFORM_BSD
        printf("jp: using bx_platform_linux|bsd\n");
        pd.ndt = wmi.info.x11.display;
        pd.nwh = (void*)(uintptr_t)wmi.info.x11.window;
#elif BX_PLATFORM_OSX
        printf("jp: using bx_platform_osx\n");
        pd.ndt = NULL;
        pd.nwh = wmi.info.cocoa.window;
#elif BX_PLATFORM_WINDOWS
        printf("jp: using bx_platform_windows\n");
        pd.ndt = NULL;
        pd.nwh = wmi.info.win.window;
#elif BX_PLATFORM_STEAMLINK
        printf("jp: using bx_platform_steamlink\n");
        pd.ndt = wmi.info.vivante.display;
        pd.nwh = wmi.info.vivante.window;
#endif // BX_PLATFORM_
        pd.context = NULL;
        pd.backBuffer = NULL;
        pd.backBufferDS = NULL;
        bgfx::renderFrame();
#else    
        printf("jp: emscripten build\n");
        pd.nwh = (void*)"#canvas";
#endif // BX_PLATFORM_EMSCRIPTEN

        engine_app->PreInit();
        Debug::Console::Init();
        bgfx::Init init;
        init.platformData = pd;
        init.resolution.width = GAME_FIXED_WIDTH;
        init.resolution.height = GAME_FIXED_HEIGHT;
        //init.resolution.reset = BGFX_RESET_VSYNC;
        init.type = bgfx::RendererType::OpenGL;
        bgfx::init(init);
        engine_app->Logf("ENGINE: bgfx initialized");
        engine_app->camera.Init();
        engine_app->NodeInitUniforms(); // uniforms must be initialized here
        // the order of this requires shader_manager to be created before mesh_manager since
        // mesh_manager has snapshots which require shaders to be loaded
        
        IO::FileSystem::FindDataPath();
        IO::FileSystem::AddDataPath({.path = "."});
        auto initialize_tasks = std::async(std::launch::async,
            [](){
                EASY_THREAD("Worker: Init managers");
                engine_app->texture_manager = new Data::TextureManager();
                engine_app->shader_manager = new Data::ShaderManager();
                engine_app->mesh_manager = new Data::MeshManager();
                engine_app->sound_manager = new Sound::SoundManager();
                engine_app->input_manager = new Input::InputManager();
                engine_app->root_node = new Scene::SceneNode();});
        bgfx::reset(GAME_FIXED_WIDTH, GAME_FIXED_HEIGHT, BGFX_RESET_VSYNC | BGFX_RESET_MSAA_X16);
        engine_app->Logf("ENGINE: resolution %ix%i...", GAME_FIXED_WIDTH, GAME_FIXED_HEIGHT);
        bgfx::setDebug(BGFX_DEBUG_TEXT);
        bgfx::setViewClear(0
            , BGFX_CLEAR_COLOR|BGFX_CLEAR_DEPTH|BGFX_CLEAR_STENCIL
            , 0x303030ff
			, 1.0f
			, 0);

        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();

        ImGui_Implbgfx_Init(255);
    #if BX_PLATFORM_WINDOWS
        ImGui_ImplSDL2_InitForD3D(window);
    #elif BX_PLATFORM_OSX
        ImGui_ImplSDL2_InitForMetal(window);
    #elif BX_PLATFORM_LINUX || BX_PLATFORM_EMSCRIPTEN
        ImGui_ImplSDL2_InitForOpenGL(window, nullptr);
    #endif // BX_PLATFORM_WINDOWS ? BX_PLATFORM_OSX ? BX_PLATFORM_LINUX ?
        // BX_PLATFORM_EMSCRIPTEN

        initialize_tasks.wait();
        engine_app->input_manager->AddSInput({.name = "Vertical", 
            .positiveSource = {.mode = Input::IM_KEYBOARD, .data = (void*)SDLK_w}, 
            .negativeSource = {.mode = Input::IM_KEYBOARD, .data = (void*)SDLK_s}});
        engine_app->input_manager->AddSInput({.name = "Horizontal", 
            .positiveSource = {.mode = Input::IM_KEYBOARD, .data = (void*)SDLK_d}, 
            .negativeSource = {.mode = Input::IM_KEYBOARD, .data = (void*)SDLK_a}});
        engine_app->input_manager->AddSInput({.name = "MouseVertical", 
            .positiveSource = {.mode = Input::IM_MOUSE, .data = (void*)0x0}, .reset = true});
        engine_app->input_manager->AddSInput({.name = "MouseHorizontal", 
            .positiveSource = {.mode = Input::IM_MOUSE, .data = (void*)0x1}, .reset = true});
        engine_app->input_manager->AddSInput({.name = "MouseClick", 
            .positiveSource = {.mode = Input::IM_MOUSE, .data = (void*)0x2}});
        engine_app->input_manager->AddSInput({.name = "MouseWVertical", 
            .positiveSource = {.mode = Input::IM_MOUSE, .data = (void*)0x3}, .reset = true});
        engine_app->input_manager->AddSInput({.name = "MouseWHorizontal", 
            .positiveSource = {.mode = Input::IM_MOUSE, .data = (void*)0x4}, .reset = true});
        UI::PrecacheUIAssets();
        if(engine_app->networking_enabled)
        {
            engine_app->Logf("ENGINE: enabling experimental gns networking system");
            Network::SteamDatagram::InitSteamDatagramConnectionSockets();
            Network::GNSClient *cli = new Network::GNSClient();
            cli->active = false;
            cli->lastAddr = SteamNetworkingIPAddr();
            cli->cstatus = Network::CS_UNKNOWN;
            cli->ctype = Network::CT_CLIENT;
            engine_app->netcontext = cli;
            engine_app->ClientInit();
        }
        else
        {
            engine_app->Logf("ENGINE: running non-networked");
            engine_app->Init();
            engine_app->ClientInit();
        }
        engine_app->Logf("ENGINE: finished initializing...");
        engine_app->Logf("ENGINE: running game %s '%s'...", engine_app->GetAppName(), engine_app->GetAppDescription());
        bgfx::setState(BGFX_STATE_DEFAULT);
        SDL_Event event;
        while(engine_app->running)
        {
            engine_app->input_manager->Update();

            while(SDL_PollEvent(&event))
            {
                if(!engine_app->focusMode)
                    ImGui_ImplSDL2_ProcessEvent(&event);
                if(engine_app->input_manager)
                    engine_app->input_manager->SDLHookEvent(&event);
                engine_app->Event(&event);
            }    
            ImGui_Implbgfx_NewFrame();
            ImGui_ImplSDL2_NewFrame();

            bgfx::setViewRect(0, 0, 0, uint16_t(GAME_FIXED_WIDTH), uint16_t(GAME_FIXED_HEIGHT));
            bgfx::touch(0);
            
            if(!engine_app->mesh_manager->RenderQueue())
            {
                EASY_BLOCK("Render Loop");
                //auto tick_tasks = std::async(std::launch::async,
                //    []() {
                //    });

                // pre tick
                engine_app->PreTick();
                engine_app->PreRender();
                if(engine_app->netcontext_s)
                    engine_app->netcontext_s->Tick();
                if(engine_app->netcontext)
                    engine_app->netcontext->Tick();
                if(!engine_app->pauseGame)
                    engine_app->root_node->Update();
                engine_app->Tick();
                
                //tick_tasks.wait();
                SDL_SetRelativeMouseMode((SDL_bool)(!engine_app->pauseGame && engine_app->focusMode));
                engine_app->root_node->RUpdate();
                engine_app->camera.Update();
                glm::mat4 view = engine_app->camera.GetViewMatrix();
                bgfx::setMarker("Render scene graph");
                bgfx::setViewTransform(0, (float*)&view, (float*)&engine_app->camera.proj);
                engine_app->sound_manager->Update();
                engine_app->root_node->Render();
                // pre render
                Debug::Console::Render();
                
                bgfx::setMarker("Render imgui");
                ImGui::NewFrame();
                if(engine_app->menu_bar_enabled)
                {
                    if(ImGui::BeginMainMenuBar())
                    {
                        ImGui::Text("Jalapeno2 %s", engine_app->GetEngineVersion());
                        if(ImGui::BeginMenu((const char*)engine_app->GetAppName()))
                        {
                            if(ImGui::MenuItem("Quit"))
                                engine_app->running = false;
                            if(ImGui::MenuItem("About"))
                                UI::im_aboutmenu_draw = true;
                            ImGui::Separator();
                            // game is meant to add their own entries here
                            ImGui::EndMenu();
                        }
                        if(ImGui::BeginMenu("Engine"))
                        {
                            ImGui::BeginDisabled(!Debug::Console::accessible);
                            ImGui::Checkbox("Console", &Debug::Console::enabled);
                            ImGui::Checkbox("ADebug", &UI::im_debugmenu_draw);
                            ImGui::EndDisabled();
                            ImGui::EndMenu();
                        }
                        if(engine_app->netcontext)
                        {
                            Network::GNSClient* cli = (Network::GNSClient*)engine_app->netcontext;
                            if(ImGui::BeginMenu("Network"))
                            {
                                ImGui::BeginDisabled(!engine_app->netcontext->active);
                                if(ImGui::MenuItem("Disconnect"))
                                {
                                    cli->Stop("Disconnect by user");
                                }
                                ImGui::EndDisabled();
                                ImGui::Separator();
                                ImGui::BeginDisabled(engine_app->netcontext->active);
                                if(ImGui::MenuItem("Reconnect"))
                                {
                                    cli->Init(cli->lastAddr);
                                }
                                ImGui::EndDisabled();
                                ImGui::EndMenu();
                            }
                        }
                        if(engine_app->netcontext_s)
                        {
                            if(ImGui::BeginMenu("Network (Server)"))
                            {
                                ImGui::BeginDisabled(!engine_app->netcontext_s->active);
                                if(ImGui::MenuItem("Stop"))
                                {
                                    // engine_app->netcontext_s->Stop();
                                }
                                ImGui::EndDisabled();
                                ImGui::EndMenu();
                            }
                        }
                        ImGui::Text("FPS: %.1f", engine_app->fps);
                    }
                    ImGui::EndMainMenuBar();
                }
                UI::IMDrawAllThings();
                engine_app->GUIRender();
                ImGui::Render();
                ImGui_Implbgfx_RenderDrawLists(ImGui::GetDrawData());
            }
            // we have to blank when renderqueue is going because the uniforms renderqueue requires overrides the preexisting ones
            // also we cant tick either because its up to Tick and Update to modify uniforms aswell
                        
            bgfx::frame();
            double currentFrame = SDL_GetPerformanceCounter();
            engine_app->delta_time = (currentFrame - lastFrame)*1000.0/(double)SDL_GetPerformanceFrequency();
            lastFrame = currentFrame;  
            EASY_BLOCK("Frame Limiting");
            SDL_Delay(__fpslimit_time());
            EASY_END_BLOCK;
            next_time += TICK_INTERVAL;
            engine_app->fps = 1000.0 / engine_app->delta_time;
            engine_app->frame_counter++;
            EASY_END_BLOCK;
        }

        if(engine_app->networking_enabled)
        {
            Network::SteamDatagram::ShutdownSteamDatagramConnectionSockets();
        }

        delete engine_app->sound_manager;
        bgfx::shutdown();
    }
    printf("goodbye... saved %i blocks\n", profiler::dumpBlocksToFile("test_profile.prof"));
    SDL_DestroyWindow(window);
    SDL_Quit();
}
#include "app.hpp"
#include <bgfx/bgfx.h>
#include <stdarg.h>
#include <easy/profiler.h>
#include "console.hpp"

void Engine::App::ClientInit()
{

}

void Engine::App::Init()
{
    EASY_FUNCTION();

}

void Engine::App::Logf(const char* format,...)
{
    EASY_FUNCTION();
    va_list arguments;
    va_start(arguments, format);
    Debug::Console::Loga(format, arguments);
    va_end(arguments);
}

void Engine::App::PreLoad(LoadType type, const char* file)
{
    EASY_FUNCTION();
    Logf("APP: Preloading %i %s...", type, file);
    switch(type)
    {
        case LT_MESH:
            mesh_manager->PrecacheLoadMesh(file);
            break;
        case LT_TEXTURE:
            texture_manager->PrecacheLoadTexture(file);
            break;
        case LT_SHADER:
            shader_manager->PrecacheLoadShader(file);
            break;
    }
}

void Engine::App::PreInit()
{
    EASY_FUNCTION();
}

void Engine::App::PreRender()
{
    EASY_FUNCTION();

}

void Engine::App::PreTick()
{
    EASY_FUNCTION();

}

void Engine::App::Tick()
{
    EASY_FUNCTION();

}

void Engine::App::GUIRender()
{  
    EASY_FUNCTION();
    int aA;
    int aB;
    aA = 0x84;
    aB = 0x87;
    if(frame_counter / 10 % 2 == 0)
    {
        aA = 0xc0;
        aB = 0xf0;
    }
    bgfx::dbgTextPrintf(20,20,aA,"No App (%s, \'%s\')",GetAppName(),GetAppDescription());
    bgfx::dbgTextPrintf(20,21,aB,"This game did not return Engine::App with enough overloads during LoadApp");
    bgfx::dbgTextPrintf(20,22,aB,"Thus the game cannot run. Contact the developer");
    bgfx::dbgTextPrintf(20,23,aB,"Developers, overload Engine::App::GUIRender() to remove this message");
}

void Engine::App::Event(SDL_Event* event)
{
    EASY_FUNCTION();
    switch(event->type)
    {
        case SDL_QUIT:
            running = false;
            break;
    }
}

void Engine::App::NodeInitUniforms()
{
    EASY_FUNCTION();
    camera.u_camera_position = bgfx::createUniform("u_camera_viewpos", bgfx::UniformType::Vec4);
    Scene::DirectionalLightNode::Init();
}

char* Engine::App::GetAppName()
{
    return "No app";
}

char* Engine::App::GetAppDescription()
{
    return "No app";
}
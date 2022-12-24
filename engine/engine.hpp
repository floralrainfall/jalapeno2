#ifndef ENGINE_ENGINE_HPP
#define ENGINE_ENGINE_HPP

#include "app.hpp"

// should be defined in project
extern Engine::App* LoadApp();
extern Engine::App* engine_app;
#define GAME_FIXED_WIDTH 1600
#define GAME_FIXED_HEIGHT 900

namespace Engine
{
    #ifdef DEBUG
    void DbgPanicMsg(const char* msg);
    #endif
};

template<typename T, typename U> constexpr size_t offset_of(U T::*member)
{
    return (char*)&((T*)nullptr->*member) - (char*)nullptr;
}

#endif
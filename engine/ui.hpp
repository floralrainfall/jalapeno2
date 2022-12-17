#ifndef ENGINE_UI_HPP
#define ENGINE_UI_HPP

#include "imgui/imgui.h"
#include "imgui/custom.hpp"

namespace UI
{
    extern bool im_aboutmenu_draw;
    extern bool im_debugmenu_draw;
    extern ImFont* roboto_regular;
    extern ImFont* roboto_black;
    void IMDrawAboutMenu();
    void IMDrawDebugMenu();
    void IMDrawConnectionStuff();
    void IMDrawAllThings();
    void PrecacheUIAssets();
}

#endif
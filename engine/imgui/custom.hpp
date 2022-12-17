#ifndef CUSTOM_HPP
#define CUSTOM_HPP

#include "imgui.h"
// https://github.com/ocornut/imgui/issues/1772#issuecomment-1127830067
namespace ImGui
{
    // Plot value over time
    // Pass FLT_MAX value to draw without adding a new value
    void	PlotVar(const char* label, float value, float scale_min = FLT_MAX, float scale_max = FLT_MAX, size_t buffer_size = 120);

    // Call this periodically to discard old/unused data
    void	PlotVarFlushOldEntries();
}

#endif
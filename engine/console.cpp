#include "console.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <bgfx/bgfx.h>
#include <bx/bx.h>
#include <algorithm>
#include <easy/profiler.h>
#include "engine.hpp"

bool Debug::Console::enabled = false;
bool Debug::Console::accessible = false;

static std::vector<Debug::ConsoleMessage*> messages;
void Debug::Console::Init()
{
    EASY_FUNCTION();
    enabled = true;
    accessible = true;
    messages = std::vector<ConsoleMessage*>();
}

void Debug::Console::Loga(const char* format, va_list vargs)
{
    EASY_FUNCTION();
    ConsoleMessage* m = (ConsoleMessage*)malloc(sizeof(ConsoleMessage));
    memset(m->message,0,sizeof(m->message));
    vsnprintf(m->message,sizeof(m->message)-1,format,vargs);
    printf("i:%i f:%i '%s'\n",messages.size(),engine_app->frame_counter,m->message);
    m->frame = engine_app->frame_counter;
    messages.push_back(m);   
}

#define TOP_OFFSET 2

void Debug::Console::Render()
{
    bgfx::dbgTextClear(0, true);
    if(enabled)
    {
        bgfx::dbgTextPrintf(0,TOP_OFFSET,0xf0,"Jalapeno2 %s CONSOLE... Frames Per Second: %f, Frame: %i",engine_app->GetEngineVersion(),engine_app->fps,engine_app->frame_counter);
        std::reverse(messages.begin(), messages.end());
        for(int i = bx::min(messages.size() - 1, GAME_FIXED_HEIGHT/8 - 16); i >= 0; i--)
        {
            ConsoleMessage* m = messages[i];
            bgfx::dbgTextPrintf(0,TOP_OFFSET+1+i,0x0f,"f:%i '%s'",m->frame,m->message);
        }
        std::reverse(messages.begin(), messages.end());
    }
}
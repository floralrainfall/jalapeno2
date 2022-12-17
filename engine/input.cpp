#include "input.hpp"
#include "engine.hpp"

Input::InputManager::InputManager()
{
       
}

void Input::InputManager::SDLHookEvent(SDL_Event* event)
{
    InputMode m;
    if(event->type == SDL_KEYDOWN || event->type == SDL_KEYUP)
        m = IM_KEYBOARD;
    if(event->type == SDL_MOUSEMOTION || event->type == SDL_MOUSEBUTTONDOWN || event->type == SDL_MOUSEBUTTONUP || event->type == SDL_MOUSEWHEEL)
        m = IM_MOUSE;
    for(auto&& f : inputFunctions)
    {
        if(f.source.mode == m)
        {
            switch(m)
            {
                case IM_KEYBOARD:
                    if(f.source.data == (void*)event->key.keysym.sym && event->type == SDL_KEYDOWN)
                    {
                        f.function();
                    }
                    break;
            }
        }
    }
    for(auto&& ikin : keyInputs)
    {
        if(ikin.source.mode == m)
        {
            switch(m)
            {
                case IM_KEYBOARD:
                    if(ikin.source.data == (void*)event->key.keysym.sym  & event->type == SDL_KEYDOWN)
                    {
                        ikin.value = 1.f;
                    }
                    if(ikin.source.data == (void*)event->key.keysym.sym  & event->type == SDL_KEYUP)
                    {
                        ikin.value = 0.f;
                    }
            }
        }
    }
    for(auto&& isin : scalarInputs)
    {
        bool clip = false;
        float value = isin.value;
        if(isin.positiveSource.mode == m)
        {
            switch(m)
            {
                case IM_KEYBOARD:
                    clip = true;
                    if(isin.positiveSource.data == (void*)event->key.keysym.sym && event->type == SDL_KEYDOWN)
                    {
                        value += 1.0f;
                    }
                    if(isin.positiveSource.data == (void*)event->key.keysym.sym && event->type == SDL_KEYUP)
                    {
                        value -= 1.0f;
                    }
                    break;
                case IM_MOUSE:
                    if(isin.positiveSource.data == (void*)0x0 && engine_app->focusMode && event->type == SDL_MOUSEMOTION) // mouse X
                    {
                        value = event->motion.xrel;
                    } else if(isin.positiveSource.data == (void*)0x1 && engine_app->focusMode && event->type == SDL_MOUSEMOTION) // mouse Y
                    {
                        value = -event->motion.yrel;
                    } else if(isin.positiveSource.data == (void*)0x2 && (event->type == SDL_MOUSEBUTTONDOWN || event->type == SDL_MOUSEBUTTONUP)) // mouse buttons
                    {
                        if(event->type == SDL_MOUSEBUTTONUP)
                            value = 0.f;
                        else
                            value = (float)event->button.button;
                    } else if(isin.positiveSource.data == (void*)0x3 && event->type == SDL_MOUSEWHEEL) // mouse wheel X
                    {
                        if(event->type == SDL_MOUSEWHEEL)
                            value = event->wheel.preciseX;
                    } else if(isin.positiveSource.data == (void*)0x4 && event->type == SDL_MOUSEWHEEL) // mouse wheel Y
                    {
                        if(event->type == SDL_MOUSEWHEEL)
                            value = event->wheel.preciseY;
                    }
                    break;
                default:
                    break;
            }
        }
        if(isin.negativeSource.mode == m)
        {
            switch(m)
            {
                case IM_KEYBOARD:
                    clip = true;
                    if(isin.negativeSource.data == (void*)event->key.keysym.sym && event->type == SDL_KEYDOWN)
                    {
                        value -= 1.0f;
                    }
                    if(isin.negativeSource.data == (void*)event->key.keysym.sym && event->type == SDL_KEYUP)
                    {
                        value += 1.0f;
                    }
                    break;
                default:
                    break;
            }
        }

        if(clip)
            isin.value = std::max(-1.f, std::min(value, 1.f));
        else
            isin.value = value;
    }
}

void Input::InputManager::AddInput(Input::KeyInput ikin)
{
    keyInputs.push_back(ikin);
}

void Input::InputManager::AddSInput(Input::ScalarInput isin)
{
    scalarInputs.push_back(isin);
}

void Input::InputManager::ResetScalars()
{
    for(auto&& isin : scalarInputs)
    {
        isin.value = 0.f;
    }
}

void Input::InputManager::Update()
{
    for(auto&& isin : scalarInputs)
    {
        if(isin.reset)
            isin.value = 0.f;
    }
}

float Input::InputManager::GetInput(const char* name)
{
    for(auto&& isin : scalarInputs)
    {
        if(strcmp(name,isin.name)==0 && isin.value != 0.f)
            return isin.value;
    }
    for(auto&& ikin : keyInputs)
    {
        if(strcmp(name,ikin.name)==0)
            return ikin.value;
    }
    return 0.f;
}

void Input::InputManager::AddInputFunction(Input::InputSource source, std::function<void()> fun)
{
    InputFunction func;
    func.function = fun;
    func.source = source;
    inputFunctions.push_back(func);
}
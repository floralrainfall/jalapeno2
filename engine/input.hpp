#ifndef ENGINE_INPUT_HPP
#define ENGINE_INPUT_HPP

#include <list>
#include <SDL2/SDL.h>
#include <functional>

namespace Input
{
    enum InputMode
    {
        IM_KEYBOARD, // TODO: implement joystick and gamepad
        IM_JOYSTICK, 
        IM_GAMEPAD,
        IM_MOUSE,
    };
    
    struct InputSource
    {
        InputMode mode;
        void* data; // IM_KEYBOARD, data == SDL_Keysym      
    };
    
    // simple on/off
    struct KeyInput
    {
        const char name[64];
        InputSource source;
        float value;
    };

    // 1.0 to -1.0 depending on positiveSource & negativeSource
    struct ScalarInput
    {
        const char name[64];
        InputSource positiveSource;
        InputSource negativeSource;
        bool reset;
        float value;
    };
    
    class InputManager
    {
    private:
        struct InputFunction
        {
            std::function<void()> function;
            InputSource source;
        };
        std::list<InputFunction> inputFunctions;
    public:          
        std::list<KeyInput> keyInputs;
        std::list<ScalarInput> scalarInputs;
        void AddInputFunction(InputSource source, std::function<void()> fun);
        void AddInput(KeyInput ikin);
        void AddSInput(ScalarInput isin);
        float GetInput(const char* name);
        InputManager();
        void SDLHookEvent(SDL_Event* event);
        void ResetScalars();
        void Update();
    };
};

#endif
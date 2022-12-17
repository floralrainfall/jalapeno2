#ifndef ENGINE_CONSOLE_HPP
#define ENGINE_CONSOLE_HPP

#include <stdarg.h>
#include <vector>

namespace Debug {
    struct ConsoleMessage
    {
        char message[512];
        int frame;
    };
    class Console
    {
    public:
        static bool enabled;
        static bool accessible;
        static void Init();
        static void Loga(const char* format, va_list vargs);
        static void Render();
    };
}

#endif
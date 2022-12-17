#ifndef ENGINE_SHADER_HPP
#define ENGINE_SHADER_HPP

#include <bgfx/bgfx.h>
#include <vector>

namespace Data
{
    struct Shader
    {
        const char* name;    

        bgfx::ShaderHandle f;
        bgfx::ShaderHandle v;
        bgfx::ProgramHandle program;
    };

    class ShaderManager
    {
    public:
        std::vector<Shader*> shaders;
        ShaderManager();
        void PrecacheLoadShader(const char* name);
        void DbgListProgramsInShader(bgfx::ShaderHandle handle);
        Shader* GetShader(const char* name);
    };
};

#endif
#include "shader.hpp"
#include <string.h>
#include <stdio.h>
#include <fstream>
#include <easy/profiler.h>
#include "engine.hpp"
#include "filesystem.hpp"

Data::ShaderManager::ShaderManager() 
{
    EASY_FUNCTION();
    PrecacheLoadShader("snapshot");
    PrecacheLoadShader("simple");
    PrecacheLoadShader("textured");
    PrecacheLoadShader("quick");
}

void Data::ShaderManager::PrecacheLoadShader(const char* name)
{
    EASY_FUNCTION();
    Shader* shader = (Shader*)malloc(sizeof(Shader));
    char shaderpath[128] = {0};

    snprintf(shaderpath, 128, "shader/%c_%s.bin",'v',name);
    engine_app->Logf("ShaderManager: Loading vert %s", shaderpath);
    std::ifstream inputv( shaderpath, std::ios::binary );
    if(!inputv)
    {
        engine_app->Logf("ShaderManager: Couldnt load vert");
        return;
    }
    std::vector<unsigned char> bufferv(std::istreambuf_iterator<char>(inputv), {});
    const bgfx::Memory* memv = bgfx::copy(bufferv.data(), bufferv.size());
    shader->v = bgfx::createShader(memv);

    snprintf(shaderpath, 128, "shader/%c_%s.bin",'f',name);
    engine_app->Logf("ShaderManager: Loading frag %s", shaderpath);
    std::ifstream inputf( shaderpath, std::ios::binary );
    if(!inputf)
    {
        engine_app->Logf("ShaderManager: Couldnt load frag");
        return;
    }
    std::vector<unsigned char> bufferf(std::istreambuf_iterator<char>(inputf), {});
    const bgfx::Memory* memf = bgfx::copy(bufferf.data(), bufferf.size());
    shader->f = bgfx::createShader(memf);

    engine_app->Logf("ShaderManager: Vtx Shaders");
    DbgListProgramsInShader(shader->v);
    engine_app->Logf("ShaderManager: Frg Shaders");
    DbgListProgramsInShader(shader->f);

    shader->program = bgfx::createProgram(shader->v,shader->f,true);
    shader->name = (const char*)malloc(strlen(name)-1);
    strcpy((char*)shader->name,name);
    shaders.push_back(shader);
    engine_app->Logf("ShaderManager: Precaching %s", name);
}

void Data::ShaderManager::DbgListProgramsInShader(bgfx::ShaderHandle handle)
{
    EASY_FUNCTION();
    bgfx::UniformHandle uniforms[64];
    int n_uniforms = bgfx::getShaderUniforms(handle, uniforms, 64);
    engine_app->Logf("DbgListProgramsInShader: %i uniforms", n_uniforms);
    for(int i = 0; i < n_uniforms; i++)
    {
        bgfx::UniformInfo info;
        bgfx::getUniformInfo(uniforms[i], info);
        engine_app->Logf("DbgListProgramsInShader: shader uniform %s, type %i", info.name, info.type);
    }
}

Data::Shader* Data::ShaderManager::GetShader(const char* name)
{
    EASY_FUNCTION();
    for(int i = 0; i < shaders.size(); i++)
    {
        if(strcmp(name,shaders[i]->name)==0)
        {
            return shaders[i];
        }
    }
    engine_app->Logf("ShaderManager: cant find shader %s", name);
    return nullptr;
}
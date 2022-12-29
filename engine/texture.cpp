#include "texture.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "engine.hpp"
#include <easy/profiler.h>
#include "filesystem.hpp"
#include <concepts>
#include <type_traits>

#define qd(x,v) (float)(x)/v

glm::vec4 Data::Texture::GetSpriteUVs(int id)
{
    int sprites_x = width/sprite_width;
    int sprites_y = height/sprite_height;

    int x = id % sprites_x;
    int y = (id / sprites_x) % sprites_y;
    return glm::vec4(qd(x*sprite_width,width), qd(y*sprite_height,height), 
                     qd(x*sprite_width+sprite_width,width), qd(y*sprite_height+sprite_height,height)); 
}

glm::vec4 Data::Texture::GetSpriteUVs(int x, int y)
{
    return glm::vec4(qd(x*sprite_width,width), qd(y*sprite_height,height), 
                     qd(x*sprite_width+sprite_width,width), qd(y*sprite_height+sprite_height,height));
}

Data::TextureManager::TextureManager()
{
    EASY_FUNCTION();
    for(int i = 0; i < sizeof(textureHandles)/sizeof(bgfx::UniformHandle); i++)
    {
        char uniformName[16];
        snprintf(uniformName, 16, "texture%i", i);
        textureHandles[i] = bgfx::createUniform(uniformName,bgfx::UniformType::Sampler);
    }
    maxTextures = sizeof(textureHandles)/sizeof(bgfx::UniformHandle);
    engine_app->Logf("TextureManager: max texture handles = %i", maxTextures);

    PrecacheLoadTexture("brand.png");
    PrecacheLoadTexture("bad_texture.png");
    unknown_texture = GetTexture("bad_texture.png");
}

void Data::TextureManager::SetTextureUniform(int stage, int i, Texture* texture)
{
    if(i > maxTextures)
        return;
    bgfx::setTexture(stage, textureHandles[i], texture->texture);
}

void Data::TextureManager::PrecacheLoadTexture(const char* name)
{
    EASY_FUNCTION();
    int iwidth;
    int iheight;
    int ichannels;
    char* idata = (char*)stbi_load(IO::FileSystem::GetDataPath(name),&iwidth,&iheight,&ichannels,4);
    if(idata)
    {
        Texture* texture = new Texture();
        texture->width = iwidth;
        texture->height = iheight;
        texture->texture = bgfx::createTexture2D(iwidth, iheight, false, 1, bgfx::TextureFormat::RGBA8, 0, bgfx::copy(idata, iwidth * iheight * 4));
        texture->name = (char*)malloc(strlen(name)+1);
        strcpy(texture->name, name);
        textures.push_back(texture);
    }
    else
    {
        engine_app->Logf("TextureManager: Couldn't load texture '%s'", name);
    }
}

void Data::TextureManager::PrecacheLoadTexture(const char* name, void* data, int data_length)
{
    EASY_FUNCTION();
    char* data_i = (char*)data;
    int resx;
    int resy;
    int comp;
    stbi_uc* data_s = stbi_load_from_memory((const stbi_uc*)data, data_length, &resx, &resy, &comp, STBI_rgb_alpha);
    if(data_s)
    {
        engine_app->Logf("TextureManager: Loaded texture '%s' %p, %ix%i %i", name, data, resx, resy, comp);
        Texture* texture = new Texture();
        texture->width = resx;
        texture->height = resy;
        texture->texture = bgfx::createTexture2D(resx, resy, false, 1, bgfx::TextureFormat::RGBA8, 0, bgfx::copy(data_s, resx * resy * 4));
        texture->name = (char*)malloc(strlen(name)+1);
        strcpy(texture->name, name);
        textures.push_back(texture);
    }
    else
    {
        engine_app->Logf("TextureManager: Couldn't load texture '%s'", name);
    }
}

void Data::TextureManager::PrecacheLoadTexture(const char* name, bgfx::TextureHandle handle, int width, int height)
{
    EASY_FUNCTION();
    Texture* texture = new Texture();
    texture->width = width;
    texture->height = height;
    texture->texture = handle;
    texture->name = (char*)malloc(strlen(name)+1);
    strcpy(texture->name, name);
    textures.push_back(texture);
}

Data::Texture* Data::TextureManager::GetTexture(const char* name, bool b)
{
    EASY_FUNCTION();
    for(int i = 0; i < textures.size(); i++)
    {
        if(strcmp(name,textures[i]->name)==0)
        {
            return textures[i];
        }
    }
    if(strcmp(name,"bad_texture.png") != 0 && b)
        return unknown_texture;
    else
        return nullptr;
}
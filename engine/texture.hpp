#ifndef ENGINE_TEXTURE_HPP
#define ENGINE_TEXTURE_HPP

#include <bgfx/bgfx.h>
#include <vector>
#include <glm/glm.hpp>

namespace Data
{
    struct Texture
    {
        bgfx::TextureHandle texture;
        int width;
        int height;

        int sprite_width;
        int sprite_height;

        glm::vec2 GetSpriteUVs(int id);

        char* name;
    };

    class TextureManager
    {
        bgfx::UniformHandle textureHandles[7];
    public:
        int maxTextures;
        std::vector<Texture*> textures;
        TextureManager();
        void PrecacheLoadTexture(const char* path);
        void PrecacheLoadTexture(const char* name, void* data, int data_length);
        void PrecacheLoadTexture(const char* name, bgfx::TextureHandle handle, int width, int height);
        void SetTextureUniform(int stage, int i, Texture* texture);
        Texture* GetTexture(const char* name);
    };
}

#endif
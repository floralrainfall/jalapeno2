#ifndef ENGINE_SOUND_HPP
#define ENGINE_SOUND_HPP
#include <AL/al.h>
#include <AL/alc.h>
#include <list>
#include "scene.hpp"

namespace Sound
{
    struct SoundBuffer
    {
        ALuint buffer;
        char name[64];
    };

    struct SoundSource
    {
        Scene::Transform transform;
        ALuint source;

        void Update();
        void Play(SoundBuffer* buffer);

        bool looping;
        int pitch;
        int gain;
    };

    class SoundManager
    {
        ALCdevice *device;
        ALCcontext *context;
        std::list<SoundSource*> sources;
        std::list<SoundBuffer*> buffers;

        ALboolean ext_enumeration;

        void DbgPrintDevices(const ALCchar *devices);
    public:
        SoundManager();
        ~SoundManager();

        void Update();

        void PreloadSound(const char* file);
        SoundBuffer* GetSound(const char* file);

        SoundSource* NewSource();
    };
}

#endif
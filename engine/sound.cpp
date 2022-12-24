#include "sound.hpp"
#include "engine.hpp"

Sound::SoundManager::SoundManager()
{
    device = alcOpenDevice(0);
    if(!device)
        engine_app->Logf("SoundManager: Failed to init alc");
    ext_enumeration = alcIsExtensionPresent(NULL, "ALC_ENUMERATION_EXT");
    if(ext_enumeration)
        DbgPrintDevices(alcGetString(NULL, ALC_DEVICE_SPECIFIER));
    context = alcCreateContext(device, NULL);
    if(!alcMakeContextCurrent(context))
        engine_app->Logf("SoundManager: couldnt make alc context current");
    
}

Sound::SoundManager::~SoundManager()
{
    for(auto&& sound_source : sources)
    {
        alDeleteSources(1, &sound_source->source);
        delete sound_source;
    }
    for(auto&& sound_buffer : buffers)
    {
        alDeleteBuffers(1, &sound_buffer->buffer);
        delete sound_buffer;
    }
    device = alcGetContextsDevice(context);
    alcMakeContextCurrent(0);
    alcDestroyContext(context);
    alcCloseDevice(device);
}

void Sound::SoundManager::DbgPrintDevices(const ALCchar *devices)
{
    const ALCchar *device = devices, *next = devices + 1;
    size_t len = 0;

    engine_app->Logf("SoundManager: Device listing:");
    while(device && *device != '\0' && next && *next != '\0')
    {
        engine_app->Logf("SoundManager: %s", device);
        len = strlen(device);
        device += (len + 1);
        next += (len + 2);
    }
}

void Sound::SoundManager::Update()
{
    glm::vec3 camera_look = engine_app->camera.front;
    glm::vec3 camera_up = engine_app->camera.up;
    ALfloat listener_orientation[] = {  camera_look.x, camera_look.y, camera_look.z, 
                                        camera_up.x, camera_up.y, camera_up.z };
    alListener3f(AL_POSITION, engine_app->camera.position.x, engine_app->camera.position.y, engine_app->camera.position.z);
    alListener3f(AL_VELOCITY, 0, 0, 0);
    alListenerfv(AL_ORIENTATION, listener_orientation);

    for(auto&& sound_source : sources)
    {
        sound_source->Update();
    }
}

void Sound::SoundSource::Update()
{
    alSourcef(source, AL_PITCH, pitch);
    alSourcef(source, AL_GAIN, gain);
    alSource3f(source, AL_POSITION, transform.pos.x, transform.pos.y, transform.pos.z);
    alSource3f(source, AL_VELOCITY, 0, 0, 0);
    alSourcei(source, AL_LOOPING, looping);
}

void Sound::SoundSource::Play(SoundBuffer* buffer)
{
    alSourcei(source, AL_BUFFER, buffer->buffer);
    alSourcePlay(source);
}

Sound::SoundSource* Sound::SoundManager::NewSource()
{
    SoundSource* source = new SoundSource;

    alGenSources((ALuint)1, &source->source);
    source->gain = 1;
    source->pitch = 1;
    source->looping = false;
    source->transform = Scene::Transform();
    source->Update();

    return source;
}

void Sound::SoundManager::PreloadSound(const char* file)
{
    
}
        
Sound::SoundBuffer* Sound::SoundManager::GetSound(const char* file)
{
    for(auto&& sound_buffer : buffers)
    {
        if(strncmp(sound_buffer->name, file, 64)==0)
        {
            return sound_buffer;
        }
    }
    return nullptr;
}
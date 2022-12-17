#include "filesystem.hpp"
#include <stdio.h>
#include "engine.hpp"
#include <iostream>
#include <fstream>

static char data_directory[255];
static char fpath[255];
char* IO::FileSystem::GetDataPath(const char* asset)
{
    for(int i = engine_app->directories.size() - 1; i >= 0; i--)
    {
        snprintf(fpath, 255, "%s/%s/%s", data_directory, engine_app->directories[i].path, asset);

        std::ifstream file_stream(fpath);
        if(file_stream.good())
        {
            file_stream.close();
            return fpath;
        }
    }

    return (char*)asset;
}

void IO::FileSystem::AddDataPath(IO::FileSystemDirectory directory)
{
    engine_app->directories.insert(engine_app->directories.begin(), directory);
}

void IO::FileSystem::FindDataPath()
{
    snprintf(data_directory,255,"data");
}
#ifndef ENGINE_FILESYSTEM_HPP
#define ENGINE_FILESYSTEM_HPP

#include <vector>

namespace IO
{
    struct FileSystemDirectory
    {
        const char* path;
    };

    class FileSystem
    {
    public:
        static char* GetDataPath(const char* asset);
        static void AddDataPath(FileSystemDirectory directory);
        static void FindDataPath();
    };
}

#endif
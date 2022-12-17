#include "save.hpp"
#include "filesystem.hpp"
#include <iostream>
#include <fstream>
#include <limits>

Save::BinaryStream::BinaryStream(const char* file)
{
    std::ifstream inputv( IO::FileSystem::GetDataPath(file), std::ios::binary );
    inputv.ignore( std::numeric_limits<std::streamsize>::max() ); // WHY does C++ require this this could just be .size() or something
    std::streamsize length = inputv.gcount();
    inputv.clear();
    inputv.seekg( 0, std::ios_base::beg );

    data = (char*)malloc(length);
    inputv.read(data, length);
    inputv.close();
    BinaryStream(data, length);
}

Save::BinaryStream::BinaryStream(char* data, int data_length)
{
    this->data = data;
    this->data_length = data_length;
    cursor_position = 0;
}

void Save::BinaryStream::ReadThing(ISaveable* save)
{
    save->Load(this);
}

void Save::BinaryStream::ReadBytes(int length, void* data)
{

    cursor_position += length;
}

void Save::BinaryStream::WriteThing(ISaveable* save)
{
    save->Save(this);
}

void Save::BinaryStream::WriteBytes(int length, void* data)
{
    
    cursor_position += length;
}

void Save::ISaveable::Save(BinaryStream* data)
{

}

void Save::ISaveable::Load(BinaryStream* data)
{
    std::list<Save::SaveDataEntry> data_entries;
    for(auto&& entry : data_entries)
    {
        switch(entry.type)
        {
            default:
            case SaveDataEntry::DEFAULT:
                break;
        }
    }
}
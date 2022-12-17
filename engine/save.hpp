#ifndef ENGINE_SAVE_HPP
#define ENGINE_SAVE_HPP

#include <list>

namespace Save
{
    struct ISaveable;

    class BinaryStream
    {
    private:
        char* data;
        int data_length;
        int cursor_position;  
    public:

        BinaryStream(const char* file);
        BinaryStream(char* data, int data_length);
        ~BinaryStream();

        void ReadThing(ISaveable* save);
        void ReadBytes(int length, void* output);

        void WriteThing(ISaveable* save);
        void WriteBytes(int length, void* output);

        void ResetCursorPos();

        void Save(const char* output);
        void Save(char* data, int data_length);
    };

    struct SaveDataEntry
    {
        enum
        {
            DEFAULT,
        } type;
        void* data;
        int length;
    };

    struct ISaveable
    {
    public:
        void Save(BinaryStream* data); // WRITE to binary stream
        void Load(BinaryStream* data); // READ from binary stream
        virtual std::list<SaveDataEntry> GetSaveDataEntries() = 0;
    };

    #define CLASS_SAVEABLE \
        virtual std::list<Save::SaveDataEntry> GetSaveDataEntries();

    #define SAVE_ENTRY_DATA(edata, elength, etype) \
        {                                          \
            Save::SaveDataEntry entry;             \
            entry.data = (void*)edata;             \
            entry.length = elength;                \
            __entry_list.push_back(entry);         \
        }

    #define START_SAVE_ENTRY_DATA(t,p) \
        std::list<SaveDataEntry> t::GetSaveDataEntries() { \
            std::list<Save::SaveDataEntry> __entry_list; \
            {   \
                std::list<Save::SaveDataEntry> parent_list; \
                __entry_list.insert(__entry_list.end(), parent_list.begin(), parent_list.end()); \
            }
            

    #define END_SAVE_ENTRY_DATA \
            return __entry_list; \
        };

    #define START_SAVE_ENTRY_DATA_2(t) \
        std::list<Save::SaveDataEntry> t::GetSaveDataEntries() { \
            std::list<Save::SaveDataEntry> __entry_list;
            

    namespace File
    {
        struct Header : public ISaveable
        {
            #define SAVE_HEADER_SIGNATURE_0 'J'
            #define SAVE_HEADER_SIGNATURE_1 'P'
            #define SAVE_HEADER_SIGNATURE_2 '2'

            char signature[3]; // 'J' 'P' '2'
            int version;
            char game_name[64];
            
            CLASS_SAVEABLE;
        };
    };
}

#endif
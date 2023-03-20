#pragma once
#include <map>
#include <string>
#include <collection.h>


namespace WinRtLibrary
{
    //private ref class Temp sealed {
    //public:
    //    Temp() {
    //    }

    //    virtual ~Temp() {
    //        int xxx = 9;
    //    }
    //};

    public ref class Class1_DuplicatesContainer sealed
    {
    public:
        Class1_DuplicatesContainer();
        virtual ~Class1_DuplicatesContainer();

        property Windows::Foundation::Collections::IMap<Platform::String^, Platform::IBox<uint64_t>^>^ DuplicatesCountByFileType {
            Windows::Foundation::Collections::IMap<Platform::String^, Platform::IBox<uint64_t>^>^ get();
        };

    private:
        Platform::Collections::Map<Platform::String^, Platform::IBox<uint64_t>^>^ duplicatesCountByFileType;

        const std::wstring packageFolder;
        const std::wstring installedFolder;
    };
}

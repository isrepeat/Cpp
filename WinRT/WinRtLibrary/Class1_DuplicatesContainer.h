#pragma once
#include <map>
#include <string>
#include <collection.h>


namespace WinRtLibrary
{
    private ref class Temp sealed {
    public:
        Temp() {
        }

        virtual ~Temp() {
            int xxx = 9;
        }
    };

    public ref class Class1_DuplicatesContainer sealed
    {
    public:
        Class1_DuplicatesContainer();
        virtual ~Class1_DuplicatesContainer();

    private:
        Platform::Collections::Map<Platform::String^, Temp^>^ duplicatesCountByFileType;
        //std::map<Platform::String^, Temp> duplicatesCountByFileType;

        const std::wstring packageFolder;
        const std::wstring installedFolder;
    };
}

#pragma once
#include <string>

namespace WinRT_DLL
{
    public ref class Class1 sealed
    {
    public:
        Class1();

    private:
        const std::wstring packageFolder;
        const std::wstring installedFolder;
    };
}

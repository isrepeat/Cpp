#pragma once
#include <string>

namespace WinRT_DLL
{
    public ref class Crasher sealed
    {
    public:
        Crasher();

        void AccessViolation();

    private:
        const std::wstring packageFolder;
        const std::wstring installedFolder;
    };
}

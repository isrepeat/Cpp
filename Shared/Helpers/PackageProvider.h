#pragma once
#include <string>

namespace H {
    class PackageProvider {
    public:
        static bool IsRunningUnderPackage();
        static std::wstring GetApplicationUserModelId();
    };
}
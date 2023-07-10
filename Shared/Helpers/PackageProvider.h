#pragma once
#include <string>

namespace H {
    class PackageProvider {
    public:
        static bool IsRunningUnderPackage();
        static std::wstring GetPackageVersion();
        static std::wstring GetPackageFamilyName();
        static std::wstring GetApplicationUserModelId();
    };
}
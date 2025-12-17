#pragma once
#include "common.h"
#include <filesystem>
#include <string>

namespace HELPERS_NS {
    class PackageProvider {
    public:
        static bool IsRunningUnderPackage();
        static std::wstring GetPackageVersion();
        static std::wstring GetPackageFamilyName();
        static std::wstring GetApplicationUserModelId();
        static std::filesystem::path GetPackageInstalledPath(std::wstring packageFamilyName);
    };
}
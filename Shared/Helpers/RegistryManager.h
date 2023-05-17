#pragma once
#include <string>
#include <Windows.h>

enum class HKey {
    ClassesRoot,
    CurrentUser,
    LocalMachine,
    Users,
};

namespace H {
    class RegistryManager {
    public:
        static void SetRegValue(HKey hKey, const std::string& path, const std::string& keyName, const std::string& value);
        static bool HasRegValue(HKey hKey, const std::string& path, const std::string& keyName);
        static std::string GetRegValue(HKey hKey, const std::string& path, const std::string& keyName);

    private:
        static HKEY ConvertToHKEY(HKey hKey);
    };
}
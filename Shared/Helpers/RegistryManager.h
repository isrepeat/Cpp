#pragma once
#include <string>

namespace H {
    // [HKEY_CURRENT_USER; REG_SZ]
    class RegistryManager {
    public:
        static bool HasRegValue(const std::string& path, const std::string& keyName);
        static std::string GetRegValue(const std::string& path, const std::string& keyName);
        static void SetRegValue(const std::string& path, const std::string& keyName, const std::string& value);
    };
}
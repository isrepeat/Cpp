#pragma once
#include "common.h"
#if COMPILE_FOR_DESKTOP
#include <string>
#include <filesystem>

namespace HELPERS_NS {
    enum class HKey {
        ClassesRoot,
        CurrentUser,
        LocalMachine,
        Users,
        CurrentConfig,
    };

    enum class RegAction {
        Get,
        Add,
        Delete,
    };


	struct RegCommand {
		RegAction regAction = RegAction::Add;
		HKey hKey = HKey::CurrentUser;
		std::filesystem::path path;
		std::wstring keyName;
		std::wstring value;
	};

	class RegistryManager {
	public:
		static bool HasRegValue(HKey hKey, const std::filesystem::path& path, const std::wstring& keyName);
		static std::wstring GetRegValue(HKey hKey, const std::filesystem::path& path, const std::wstring& keyName);
		static void SetRegValue(HKey hKey, const std::filesystem::path& path, const std::wstring& keyName, const std::wstring& value);
		static void DeleteRegValue(HKey hKey, const std::filesystem::path& path, const std::wstring& keyName);

		static bool HasRegValue(RegCommand regCommand);
		static std::wstring GetRegValue(RegCommand regCommand);
		
		static void SetRegValue(RegCommand regCommand);
		static void SetRegValuesAdmin(std::vector<RegCommand> regCommands);

		static void DeleteRegValue(RegCommand regCommand);
		static void DeleteRegValuesAdmin(std::vector<RegCommand> regCommands);

	private:
		static std::wstring MakeRegShellCommand(RegCommand regCommand);
		static void ExecuteRegCommandWithShellAdmin(std::wstring regShellCommand);
	};
}
#endif
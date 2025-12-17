#include "RegistryManager.h"
#if COMPILE_FOR_DESKTOP
#include <MagicEnum/MagicEnum.h>
#include "Helpers.h"
#include "Logger.h"

#include <vector>
#include <format>
#include <ranges>

namespace HELPERS_NS {
	namespace Tools {
		HKEY ConvertHKeyToWin32HKEY(HKey hKey) {
			switch (hKey) {
			case HKey::ClassesRoot:
				return HKEY_CLASSES_ROOT;
			case HKey::CurrentUser:
				return HKEY_CURRENT_USER;
			case HKey::LocalMachine:
				return HKEY_LOCAL_MACHINE;
			case HKey::Users:
				return HKEY_USERS;
			case HKey::CurrentConfig:
				return HKEY_CURRENT_CONFIG;
			}
			return nullptr;
		}

		std::wstring ConvertHKeyToCmd(HKey hKey) {
			switch (hKey) {
			case HKey::ClassesRoot:
				return L"HKCR";
			case HKey::CurrentUser:
				return L"HKCU";
			case HKey::LocalMachine:
				return L"HKLM";
			case HKey::Users:
				return L"HKU";
			case HKey::CurrentConfig:
				return L"HKCC";
			}
			return L"";
		}

		std::wstring ConvertRegActionToCmd(RegAction regAction) {
			switch (regAction) {
			case RegAction::Get:
				return L"GET";
			case RegAction::Add:
				return L"ADD";
			case RegAction::Delete:
				return L"DELETE";
			}
			return L"";
		}
	} // namespace Tools


	bool RegistryManager::HasRegValue(HKey hKey, const std::filesystem::path& path, const std::wstring& keyName) {
		LOG_FUNCTION_ENTER("HasRegValue(hKey = {}, path = {}, keyName = {})"
			, MagicEnum::ToString(hKey)
			, path
			, H::WStrToStr(keyName)
		);
		return RegistryManager::HasRegValue({ RegAction::Get, hKey, path, keyName });
	}


	std::wstring RegistryManager::GetRegValue(HKey hKey, const std::filesystem::path& path, const std::wstring& keyName) {
		LOG_FUNCTION_ENTER("GetRegValue(hKey = {}, path = {}, keyName = {})"
			, MagicEnum::ToString(hKey)
			, path
			, H::WStrToStr(keyName)
		);
		return RegistryManager::GetRegValue({ RegAction::Get, hKey, path, keyName });
	}


	void RegistryManager::SetRegValue(HKey hKey, const std::filesystem::path& path, const std::wstring& keyName, const std::wstring& value) {
		LOG_FUNCTION_ENTER("SetRegValue(hKey = {}, path = {}, keyName = {}, value = {})"
			, MagicEnum::ToString(hKey)
			, path
			, H::WStrToStr(keyName)
			, H::WStrToStr(value)
		);
		RegistryManager::SetRegValue({ RegAction::Add, hKey, path, keyName, value });
	}


	void RegistryManager::DeleteRegValue(HKey hKey, const std::filesystem::path& path, const std::wstring& keyName) {
		LOG_FUNCTION_ENTER("DeleteRegValue(hKey = {}, path = {}, keyName = {})"
			, MagicEnum::ToString(hKey)
			, path
			, H::WStrToStr(keyName)
		);
		RegistryManager::DeleteRegValue({ RegAction::Delete, hKey, path, keyName });
	}



	bool RegistryManager::HasRegValue(RegCommand regCommand) {
		LOG_FUNCTION_ENTER("HasRegValue(regCommand)");
		if (RegistryManager::GetRegValue(regCommand).empty()) {
			return false;
		}
		return true;
	}


	std::wstring RegistryManager::GetRegValue(RegCommand regCommand) {
		auto& [_regAction, hKey, path, keyName, value] = regCommand;
		LOG_FUNCTION_ENTER("GetRegValue([hKey = {}, path = {}, keyName = {})"
			, MagicEnum::ToString(hKey)
			, path
			, H::WStrToStr(keyName)
		);

		std::vector<wchar_t> buffer(255);
		DWORD dataSize = static_cast<DWORD>(buffer.size() * sizeof(wchar_t));
		LSTATUS status;

		do {
			status = ::RegGetValueW(
				Tools::ConvertHKeyToWin32HKEY(hKey),
				path.wstring().c_str(),
				keyName.c_str(),
				RRF_RT_REG_SZ | RRF_ZEROONFAILURE | RRF_SUBKEY_WOW6464KEY,
				nullptr,
				buffer.data(),
				&dataSize
			);

			if (status == ERROR_MORE_DATA) {
				buffer.resize(dataSize / sizeof(wchar_t));
			}
		} while (status == ERROR_MORE_DATA);

		if (status != ERROR_SUCCESS) {
			LOG_FAILED(::HRESULT_FROM_WIN32(status));
			LOG_DEBUG_D("Not found reg value");
			return L"";
		}

		return H::VecToWStr(buffer);
	}



	void RegistryManager::SetRegValue(RegCommand regCommand) {
		auto& [_regAction, hKey, path, keyName, value] = regCommand;
		LOG_FUNCTION_ENTER("SetRegValue([hKey = {}, path = {}, keyName = {}, value = {}]"
			, MagicEnum::ToString(hKey)
			, path
			, H::WStrToStr(keyName)
			, H::WStrToStr(value)
		);

		LSTATUS status = ::RegSetKeyValueW(
			Tools::ConvertHKeyToWin32HKEY(hKey),
			path.wstring().c_str(),
			keyName.c_str(),
			REG_SZ,
			value.c_str(),
			static_cast<DWORD>((value.size() + 1) * sizeof(wchar_t))
		);

		if (status != ERROR_SUCCESS) {
			LOG_FAILED(::HRESULT_FROM_WIN32(status));
			LOG_WARNING_D("Can't set reg value. Status = {}.", status);
		}
	}


	void RegistryManager::SetRegValuesAdmin(std::vector<RegCommand> regCommands) {
		LOG_FUNCTION_ENTER("SetRegValuesAdmin(regCommands)");

		if (regCommands.empty()) {
			LOG_WARNING_D("skip setting reg values because 'regCommands' is empty");
			return;
		}

		std::wstring regShellCommand = L"/c ";
		regShellCommand += RegistryManager::MakeRegShellCommand(regCommands.at(0));

		for (auto& regCommand : std::ranges::drop_view{ regCommands, 1 }) {
			regShellCommand += L"&";
			regShellCommand += RegistryManager::MakeRegShellCommand(regCommand);
		}

		RegistryManager::ExecuteRegCommandWithShellAdmin(regShellCommand);
	}



	void RegistryManager::DeleteRegValue(RegCommand regCommand) {
		auto& [_regAction, hKey, path, keyName, value] = regCommand;
		LOG_FUNCTION_ENTER("DeleteRegValue([hKey = {}, path = {}, keyName = {})"
			, MagicEnum::ToString(hKey)
			, path
			, H::WStrToStr(keyName)
		);

		HKEY subKey = nullptr;

		LSTATUS status = ::RegOpenKeyExW(
			Tools::ConvertHKeyToWin32HKEY(hKey),
			path.wstring().c_str(),
			0,
			KEY_SET_VALUE,
			&subKey
		);

		if (status != ERROR_SUCCESS) {
			LOG_FAILED(::HRESULT_FROM_WIN32(status));
			LOG_WARNING_D("Can't open key for delete: Status = {}.", status);
			return;
		}

		status = ::RegDeleteValueW(subKey, keyName.c_str());

		if (status != ERROR_SUCCESS) {
			LOG_FAILED(::HRESULT_FROM_WIN32(status));
			LOG_WARNING_D("Can't delete value: Status = {}.", status);
		}

		::RegCloseKey(subKey);
	}


	void RegistryManager::DeleteRegValuesAdmin(std::vector<RegCommand> regCommands) {
		LOG_FUNCTION_ENTER("DeleteRegValuesAdmin(regCommands)");

		if (regCommands.empty()) {
			LOG_WARNING_D("skip setting reg values because 'regCommands' is empty");
			return;
		}

		std::wstring regShellCommand = L"/c ";
		regShellCommand += RegistryManager::MakeRegShellCommand(regCommands.at(0));

		for (auto& regCommand : std::ranges::drop_view{ regCommands, 1 }) {
			regShellCommand += L"&";
			regShellCommand += RegistryManager::MakeRegShellCommand(regCommand);
		}

		RegistryManager::ExecuteRegCommandWithShellAdmin(regShellCommand);
	}



	std::wstring RegistryManager::MakeRegShellCommand(RegCommand regCommand) {
		auto& [regAction, hKey, path, keyName, value] = regCommand;
		LOG_FUNCTION_ENTER("MakeRegShellCommand([hKey = {}, path = {}, keyName = {}, value = {}]"
			, MagicEnum::ToString(regAction)
			, MagicEnum::ToString(hKey)
			, path
			, H::WStrToStr(keyName)
			, H::WStrToStr(value)
		);

		std::wstring regShellCommand = std::format(L" REG {} \"{}\\{}\" /f "
			, Tools::ConvertRegActionToCmd(regAction)
			, Tools::ConvertHKeyToCmd(hKey)
			, path.wstring()
		);

		if (!value.empty()) {
			regShellCommand += std::format(L"/v \"{}\" /t REG_SZ /d \"{}\" "
				, keyName
				, value
			);
		}
		return regShellCommand;
	}


	void RegistryManager::ExecuteRegCommandWithShellAdmin(std::wstring regShellCommand) {
		LOG_FUNCTION_ENTER(L"ExecuteRegCommandWithShellAdmin(regShellCommand):\n{}\n", regShellCommand);
		
		DWORD exitCode;
		H::ExecuteCommandLineW(regShellCommand.c_str(), true, SW_HIDE, &exitCode);

		if (exitCode != 0) {
			LOG_ERROR_D(L"Execute shell command failed:\n"
				" command: \"{}\"\n"
				" exitCode = {}\n"
				, regShellCommand
				, exitCode
			);
		}
	}
}
#endif
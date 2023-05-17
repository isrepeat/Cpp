#include "RegistryManager.h"
#include "Helpers.h"
#include <vector>

namespace H {
	// TODO: rewrite with wstring (be careful with double length of character)
	void RegistryManager::SetRegValue(HKey hKey, const std::string& path, const std::string& keyName, const std::string& value) {
		auto status = RegSetKeyValueA(ConvertToHKEY(hKey), path.c_str(), keyName.c_str(), REG_SZ, value.c_str(), value.size());
		if (status != S_OK) {
			//WriteDebug("Status = %d", status);
		}
	}

	bool RegistryManager::HasRegValue(HKey hKey, const std::string& path, const std::string& keyName) {
		std::vector<char> buff(255);
		DWORD sz = buff.size();

		return RegGetValueA(ConvertToHKEY(hKey), path.c_str(), keyName.c_str(), RRF_RT_ANY | RRF_SUBKEY_WOW6464KEY, NULL, buff.data(), &sz) == S_OK;
	}

	std::string RegistryManager::GetRegValue(HKey hKey, const std::string& path, const std::string& keyName) {
		std::vector<char> buff(255);
		DWORD sz = buff.size();

		auto status = RegGetValueA(ConvertToHKEY(hKey), path.c_str(), keyName.c_str(), RRF_RT_ANY | RRF_SUBKEY_WOW6464KEY, NULL, buff.data(), &sz);
		if (status == S_OK) {
			return H::VecToStr(buff);
		}

		//WriteDebug("Status = %d", status);
		return "";
	}

	HKEY RegistryManager::ConvertToHKEY(HKey hKey) {
		switch (hKey) {
		case HKey::ClassesRoot:
			return HKEY_CLASSES_ROOT;
		case HKey::CurrentUser:
			return HKEY_CURRENT_USER;
		case HKey::LocalMachine:
			return HKEY_LOCAL_MACHINE;
		case HKey::Users:
			return HKEY_USERS;
		}

		return nullptr;
	}
};
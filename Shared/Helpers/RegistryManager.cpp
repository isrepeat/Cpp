#include "RegistryManager.h"
#include "HString.h"
#include <vector>
#include "Logger.h"
#include "Helpers.h"

namespace H {
	bool RegistryManager::HasRegValue(const std::string& path, const std::string& keyName) {
		LOG_FUNCTION_ENTER("HasRegValue({}, {}) ...", path, keyName);

		std::vector<char> buff(255);
		DWORD sz = buff.size();

		auto status = RegGetValueA(HKEY_CURRENT_USER, path.c_str(), keyName.c_str(), RRF_RT_ANY | RRF_SUBKEY_WOW6464KEY, NULL, buff.data(), &sz);
		if (status != S_OK) {
			LOG_DEBUG_D("Not found reg value");
			return false;
		}

		return true;
	}

	std::string RegistryManager::GetRegValue(const std::string& path, const std::string& keyName) {
		LOG_FUNCTION_ENTER("GetRegValue({}, {}) ...", path, keyName);

		std::vector<char> buff(255);
		DWORD sz = buff.size();

		auto status = RegGetValueA(HKEY_CURRENT_USER, path.c_str(), keyName.c_str(), RRF_RT_ANY | RRF_SUBKEY_WOW6464KEY, NULL, buff.data(), &sz);
		if (status != S_OK) {
			LOG_WARNING_D("Can't get reg value. Status = {}", status);
			return "";
		}
		
		return H::VecToStr(buff);
	}

	// TODO: rewrite with wstring (be careful with double length of character)
	void RegistryManager::SetRegValue(const std::string& path, const std::string& keyName, const std::string& value) {
		LOG_FUNCTION_ENTER("SetRegValue({}, {}) ...", path, keyName);

		auto status = RegSetKeyValueA(HKEY_CURRENT_USER, path.c_str(), keyName.c_str(), REG_SZ, value.c_str(), value.size());
		if (status != S_OK) {
			LOG_WARNING_D("Can't set reg value. Status = {}", status);
		}
	}
}
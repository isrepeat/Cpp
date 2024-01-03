#include "SettingsKeeper.h"
#include <string>

namespace {
	const std::string uuidAppKey = "uuidAppKey";
}
//const std::wstring& SettingsKeeper::uuidAppKey() {
//	static std::wstring uuidAppKey = L"uuidAppKey";
//	return uuidAppKey;
//}

//SettingsKeeper::SettingsKeeper()
//{
//}

const int localValue = 17;


SettingsKeeper& SettingsKeeper::GetInstance() {
	static SettingsKeeper instance;
	return instance;
}

bool SettingsKeeper::HasAppUUID() {
	if (localValue == 17) {
		int xx = 9;
	}
	return uuidAppKey == "uuidAppKey";
}
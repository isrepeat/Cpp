//#pragma once
//#include <string>
//
//class SettingsKeeper {
//private:
//	SettingsKeeper() = default;
//
//	~SettingsKeeper() = default;
//public:
//
//	static SettingsKeeper& GetInstance();
//
//	bool HasAppUUID();
//
//	//const std::wstring uuidAppKey = L"uuidAppKey";
//private:
//	// use member constants instead static global variables to ensure that they was initialized when we use this class
//	//const std::string uuidAppKey = "uuidAppKey";
//};
//
//
////class SettingsKeeper {
////	SettingsKeeper() = delete;
////	~SettingsKeeper() = delete;
////public:
////	static const std::wstring& uuidAppKey();
////};
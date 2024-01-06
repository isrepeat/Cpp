#include "WAppConstants.h"
#include "SettingsKeeper.h"

const std::wstring WAppConstants::appUuid = SettingsKeeper::GetInstance().HasAppUUID() ? L"has" : L"no";
//const std::wstring WAppConstants::appUuid = SettingsKeeper::uuidAppKey();
//const std::wstring WAppConstants::appUuid = SettingsKeeper::GetInstance().uuidAppKey;
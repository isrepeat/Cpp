#include "CrashHandler.h"
#include "CrashHandlerWinRt.h"

namespace WinRT_DLL {
	CrashHandlerWinRt^ CrashHandlerWinRt::instance = nullptr;

	CrashHandlerWinRt::CrashHandlerWinRt(Platform::String^ appCenterId, Platform::String^ appVersion, Platform::String^ appUuid, Platform::String^ minidumpWriterProtocolName) {
		H::CrashHandlerSingleton::CreateInstance(minidumpWriterProtocolName->Data(), Windows::Storage::ApplicationData::Current->LocalFolder->Path->Data(), appCenterId->Data(), appVersion->Data(), appUuid->Data());
		H::CrashHandlerSingleton::GetInstance().SetProtocolCommandArgs({
			{L"-isUWP", L""},
			{L"-debug", L""}
			});
		H::CrashHandlerSingleton::GetInstance().SetCrashCallback([this] {
			this->CrashEvent();
			});
	}

	CrashHandlerWinRt^ CrashHandlerWinRt::CreateInstance(Platform::String^ appCenterId, Platform::String^ appVersion, Platform::String^ appUuid, Platform::String^ minidumpWriterProtocolName) {
		if (!instance) {
			instance = ref new CrashHandlerWinRt(appCenterId, appVersion, appUuid, minidumpWriterProtocolName);
		}
		return instance;
	}

	CrashHandlerWinRt^ CrashHandlerWinRt::GetInstance() {
		return instance;
	}
}
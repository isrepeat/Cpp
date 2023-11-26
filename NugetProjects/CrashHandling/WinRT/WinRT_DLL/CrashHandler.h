#pragma once
#include "../../../../Shared/HelpersWinRT/Singleton.h"
#include <functional>
#include <utility>
#include <vector>
#include <string>

namespace H {
	class CrashHandler {
	public:
		~CrashHandler() = default;
		CrashHandler(std::wstring runProtocol, std::wstring packageFolder, std::wstring appCenterId, std::wstring appVersion, std::wstring appUuid);

		void SetProtocolCommandArgs(std::vector<std::pair<std::wstring, std::wstring>> protocolCommandArgs);
		void SetCrashCallback(std::function<void()> crashCallback);

	private:
		std::wstring runProtocol;
		std::function<void()> crashCallback = nullptr;
		std::vector<std::pair<std::wstring, std::wstring>> protocolCommandArgs;
	};

	using CrashHandlerSingleton = SingletonUnscoped<CrashHandler, std::wstring, std::wstring, std::wstring, std::wstring, std::wstring>;
}
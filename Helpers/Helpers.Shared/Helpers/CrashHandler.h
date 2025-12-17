//#pragma once
//#include "common.h"
//#ifdef CRASH_HANDLING_SUPPORT
//#include "HWindows.h"
//#include "Singleton.hpp"
//#include <functional>
//#include <utility>
//#include <vector>
//#include <string>
//
//namespace HELPERS_NS {
//	class CrashHandler {
//	public:
//		CrashHandler(std::wstring runProtocol, std::wstring appCenterId, std::wstring appUuid);
//		~CrashHandler() = default;
//
//		void SetProtocolCommandArgs(std::vector<std::pair<std::wstring, std::wstring>> protocolCommandArgs);
//		void SetCrashCallback(std::function<void()> crashCallback);
//
//	private:
//		std::wstring runProtocol;
//		std::function<void()> crashCallback = nullptr;
//		std::vector<std::pair<std::wstring, std::wstring>> protocolCommandArgs;
//	};
//	
//	using CrashHandlerSingleton = Singleton<CrashHandler>;
//}
//#endif
#include <Windows.h>
#include "CrashHandler.h"
#include <CrashHandling/CrashHandling.h>

static CrashHandling::AdditionalInfo additionalInfo;

namespace H {
	CrashHandler::CrashHandler(std::wstring runProtocol, std::wstring packageFolder, std::wstring appCenterId, std::wstring appVersion, std::wstring appUuid) {
		additionalInfo.packageFolder = packageFolder;
		additionalInfo.appCenterId = appCenterId;
		additionalInfo.appVersion = appVersion;
		additionalInfo.appUuid = appUuid;
		this->runProtocol = runProtocol;

		CrashHandling::RegisterDefaultCrashHandler([this](EXCEPTION_POINTERS* pep) {
			std::wstring exceptionMsg;
			switch (pep->ExceptionRecord->ExceptionCode) {
			case EXCEPTION_ACCESS_VIOLATION:
				exceptionMsg = L"EXCEPTION_ACCESS_VIOLATION";
				break;
			case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
				exceptionMsg = L"EXCEPTION_ARRAY_BOUNDS_EXCEEDED";
				break;
			case EXCEPTION_DATATYPE_MISALIGNMENT:
				exceptionMsg = L"EXCEPTION_DATATYPE_MISALIGNMENT";
				break;
			case EXCEPTION_FLT_DENORMAL_OPERAND:
				exceptionMsg = L"EXCEPTION_FLT_DENORMAL_OPERAND";
				break;
			case EXCEPTION_FLT_DIVIDE_BY_ZERO:
				exceptionMsg = L"EXCEPTION_FLT_DIVIDE_BY_ZERO";
				break;
			case EXCEPTION_FLT_INEXACT_RESULT:
				exceptionMsg = L"EXCEPTION_FLT_INEXACT_RESULT";
				break;
			case EXCEPTION_FLT_INVALID_OPERATION:
				exceptionMsg = L"EXCEPTION_FLT_INVALID_OPERATION";
				break;
			case EXCEPTION_FLT_OVERFLOW:
				exceptionMsg = L"EXCEPTION_FLT_OVERFLOW";
				break;
			case EXCEPTION_FLT_STACK_CHECK:
				exceptionMsg = L"EXCEPTION_FLT_STACK_CHECK";
				break;
			case EXCEPTION_ILLEGAL_INSTRUCTION:
				exceptionMsg = L"EXCEPTION_ILLEGAL_INSTRUCTION";
				break;
			case EXCEPTION_IN_PAGE_ERROR:
				exceptionMsg = L"EXCEPTION_IN_PAGE_ERROR";
				break;
			case EXCEPTION_INT_DIVIDE_BY_ZERO:
				exceptionMsg = L"EXCEPTION_INT_DIVIDE_BY_ZERO";
				break;
			case EXCEPTION_INT_OVERFLOW:
				exceptionMsg = L"EXCEPTION_INT_OVERFLOW";
				break;
			}

			auto backtrace = CrashHandling::GetBacktrace(0);
			auto backtraceStr = CrashHandling::BacktraceToString(backtrace);

			if (this->crashCallback) {
				this->crashCallback();
			}

			additionalInfo.backtrace = backtraceStr;
			additionalInfo.exceptionMsg = exceptionMsg;
			CrashHandling::GenerateCrashReport(pep, additionalInfo, this->runProtocol, this->protocolCommandArgs, [](const std::wstring& protcolWithParams) {
				auto uri = ref new Windows::Foundation::Uri(ref new Platform::String(protcolWithParams.c_str()));
				Windows::System::Launcher::LaunchUriAsync(uri);
				});

			CrashHandlerSingleton::DeleteInstance();
			});
	}

	void CrashHandler::SetProtocolCommandArgs(std::vector<std::pair<std::wstring, std::wstring>> protocolCommandArgs) {
		this->protocolCommandArgs = protocolCommandArgs;
	}

	void CrashHandler::SetCrashCallback(std::function<void()> crashCallback) {
		this->crashCallback = crashCallback;
	}
}
#include "CrashHandling.h"
#include "MiniDumpMessages.h"
#include <ComAPI/ComAPI.h>
#include <Helpers/Helpers.h>
#include <Helpers/PackageProvider.h>
#include <Helpers/Channel.h>
#include <Helpers/CrashInfo.h>
#include <dbghelp.h>
#pragma comment (lib, "dbghelp.lib" )


namespace CrashHandling {
	std::function<void(EXCEPTION_POINTERS*, ExceptionType)> gCrashCallback = nullptr;
	Channel<MiniDumpMessages> channelMinidump;
	const uint32_t ClrException = 0xE0434352;
	const uint32_t CppException = 0xE06D7363;
	bool wasCrashException = false;
	bool handledCrashException = false;


	LONG WINAPI DefaultVectoredExceptionHandlerFirst(EXCEPTION_POINTERS* pep);
	LONG WINAPI DefaultVectoredExceptionHandlerLast(EXCEPTION_POINTERS* pep);
	LONG WINAPI DefaultUnhandledExceptionFilter(EXCEPTION_POINTERS* pep);


	CRASH_HANDLING_API Backtrace GetBacktrace(int SkipFrames) {
		constexpr int TRACE_MAX_STACK_FRAMES = 99;
		void* stack[TRACE_MAX_STACK_FRAMES];
		ULONG hash;

		////int numFrames = CaptureStackBackTrace(SkipFrames + 1, TRACE_MAX_STACK_FRAMES, stack, &hash);

		int attempts = 3;
		int numFrames = 0;
		while ((numFrames = CaptureStackBackTrace(SkipFrames + 1, TRACE_MAX_STACK_FRAMES, stack, &hash)) == 0) {
			if (--attempts == 0)
				break;
			//printf("numFrames == 0, try again [attempts left %d]", attempts);
		}

		if (numFrames == 0) {
			//printf("BackTrace(int) numFrames = 0\n");
			return {};
		}

		Backtrace backtrace;
		std::wstring prevModuleFilename = L"";

		for (int i = 0; i < numFrames; ++i) {
			void* moduleBaseVoid = nullptr;
			RtlPcToFileHeader(stack[i], &moduleBaseVoid);
			auto moduleBase = (const unsigned char*)moduleBaseVoid;
			constexpr auto MODULE_BUF_SIZE = 4096U;
			wchar_t modulePath[MODULE_BUF_SIZE];
			if (moduleBase != nullptr) {
				GetModuleFileNameW((HMODULE)moduleBase, modulePath, MODULE_BUF_SIZE);
				std::wstring moduleFilepath{ modulePath };

				int n = moduleFilepath.rfind(L"\\");
				if (n != std::wstring::npos) {
					auto moduleFilename = moduleFilepath.substr(n + 1);
					if (moduleFilename != prevModuleFilename) {
						prevModuleFilename = moduleFilename;
						backtrace.push_back({ moduleFilename, {} });
					}
					backtrace.back().second.push_back({ moduleFilename, std::uint32_t((unsigned char*)stack[i] - moduleBase) });
				}
			}
		}
		return backtrace;
	}

	CRASH_HANDLING_API std::wstring BacktraceToString(const Backtrace& backtrace) {
		std::wstring backtraceStr;

		for (auto& [modulename, backtraceFrames] : backtrace) {
			for (auto& backtraceFrame : backtraceFrames) {
				std::vector<wchar_t> buff(1024, '\0'); // mb need more for complex programs
				swprintf_s(buff.data(), buff.size(), L"%s 0x%08lx \n", backtraceFrame.moduleName.c_str(), backtraceFrame.RVA);

				backtraceStr += H::VecToWStr(buff);
			}
		}

		return backtraceStr;
	}


	CRASH_HANDLING_API void RegisterVectorHandler(PVECTORED_EXCEPTION_HANDLER handler) {
		AddVectoredExceptionHandler(0, handler);
	}

	CRASH_HANDLING_API void RegisterDefaultCrashHandler(std::function<void(EXCEPTION_POINTERS*, ExceptionType)> crashCallback) {
		gCrashCallback = crashCallback;
		AddVectoredExceptionHandler(1, DefaultVectoredExceptionHandlerFirst); // additional guard for cathing c++ exception within clr environment
		AddVectoredExceptionHandler(0, DefaultVectoredExceptionHandlerLast);
		SetUnhandledExceptionFilter(&DefaultUnhandledExceptionFilter);
	}

	CRASH_HANDLING_API void GenerateCrashReport(EXCEPTION_POINTERS* pExceptionPtrs, const AdditionalInfo& additionalInfo, const std::wstring& runProtocolMinidumpWriter,
		const std::vector<std::pair<std::wstring, std::wstring>>& commandArgs, std::function<void(const std::wstring&)> callbackToRunProtocol) {
		auto processId = GetCurrentProcessId();
		auto threadId = GetCurrentThreadId();

		std::vector<std::pair<std::wstring, std::wstring>> params = {
			{L"-processId", std::to_wstring(processId)},
		};
		params.insert(params.end(), commandArgs.begin(), commandArgs.end());

		if (callbackToRunProtocol) {
			std::wstring protcolWithParams = runProtocolMinidumpWriter + L":\"" + H::CreateStringParams(params) + L"\"";
			callbackToRunProtocol(protcolWithParams);
		}
		else {
			std::wstring protcolWithParams = L"/c start " + runProtocolMinidumpWriter + L":\"" + H::CreateStringParams(params) + L"\"";
			H::ExecuteCommandLine(protcolWithParams, false, SW_HIDE);
		}

		std::wstring packageFolder = H::PackageProvider::IsRunningUnderPackage() ? ComApi::GetPackageFolder() : H::ExePathW();

		if (additionalInfo.appVersion.empty()) {
			const_cast<AdditionalInfo&>(additionalInfo).appVersion = H::PackageProvider::GetPackageVersion();
		}

		try {
			channelMinidump.Open(additionalInfo.channelName, [=](Channel<MiniDumpMessages>::ReadFunc Read, Channel<MiniDumpMessages>::WriteFunc Write) {
				auto reply = Read();
				switch (reply.type)
				{
				case MiniDumpMessages::Connect: {
					std::string strData = H::WStrToStr(packageFolder);
					Write({ strData.begin(), strData.end() }, MiniDumpMessages::PackageFolder);

					strData = H::WStrToStr(additionalInfo.appCenterId);
					Write({ strData.begin(), strData.end() }, MiniDumpMessages::AppCenterId);

					strData = H::WStrToStr(additionalInfo.appVersion);
					Write({ strData.begin(), strData.end() }, MiniDumpMessages::AppVersion);

					strData = H::WStrToStr(additionalInfo.appUuid);
					Write({ strData.begin(), strData.end() }, MiniDumpMessages::AppUuid);

					strData = H::WStrToStr(additionalInfo.backtrace);
					Write({ strData.begin(), strData.end() }, MiniDumpMessages::Backtrace);

					strData = H::WStrToStr(additionalInfo.exceptionMsg);
					Write({ strData.begin(), strData.end() }, MiniDumpMessages::ExceptionMessage);

					auto crashInfo = std::make_shared<CrashInfo>();
					crashInfo->threadId = threadId;
					FillCrashInfoWithExceptionPointers(crashInfo, pExceptionPtrs);
					auto serializedData = SerializeCrashInfo(crashInfo);
					Write(std::move(serializedData), MiniDumpMessages::ExceptionInfo);
					break;
				}
				}

				return true;
				}, 30'000);

			channelMinidump.WaitFinishSendingMessage(MiniDumpMessages::ExceptionInfo);
		}
		catch (...) {
			Dbreak;
		}
	}

	LONG WINAPI DefaultVectoredExceptionHandlerFirst(EXCEPTION_POINTERS* pExceptionPtrs) {
		switch (pExceptionPtrs->ExceptionRecord->ExceptionCode) {
		case EXCEPTION_ACCESS_VIOLATION:
		case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
		case EXCEPTION_DATATYPE_MISALIGNMENT:
		case EXCEPTION_FLT_DENORMAL_OPERAND:
		case EXCEPTION_FLT_DIVIDE_BY_ZERO:
		case EXCEPTION_FLT_INEXACT_RESULT:
		case EXCEPTION_FLT_INVALID_OPERATION:
		case EXCEPTION_FLT_OVERFLOW:
		case EXCEPTION_FLT_STACK_CHECK:
		case EXCEPTION_ILLEGAL_INSTRUCTION:
		case EXCEPTION_IN_PAGE_ERROR:
		case EXCEPTION_INT_OVERFLOW:
			wasCrashException = true;
			break;

		case ClrException: {
			int xxx = 9;
			break;
		}
		case CppException: {
			int xxx = 9;
			break;
		}
		}

		return EXCEPTION_CONTINUE_SEARCH;
	}


	LONG WINAPI DefaultVectoredExceptionHandlerLast(EXCEPTION_POINTERS* pExceptionPtrs) {
		if (handledCrashException) {
			printf("DefaultVectoredExceptionHandlerLast() crash exception already handled, ignore ... \n");
			return EXCEPTION_CONTINUE_SEARCH; // ignore next exception
		}

		std::wstring exceptionMsg;
		switch (pExceptionPtrs->ExceptionRecord->ExceptionCode) {
		case EXCEPTION_ACCESS_VIOLATION:
		case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
		case EXCEPTION_DATATYPE_MISALIGNMENT:
		case EXCEPTION_FLT_DENORMAL_OPERAND:
		case EXCEPTION_FLT_DIVIDE_BY_ZERO:
		case EXCEPTION_FLT_INEXACT_RESULT:
		case EXCEPTION_FLT_INVALID_OPERATION:
		case EXCEPTION_FLT_OVERFLOW:
		case EXCEPTION_FLT_STACK_CHECK:
		case EXCEPTION_ILLEGAL_INSTRUCTION:
		case EXCEPTION_IN_PAGE_ERROR:
		case EXCEPTION_INT_DIVIDE_BY_ZERO:
		case EXCEPTION_INT_OVERFLOW:
			break;

		case ClrException: {
			if (wasCrashException) {
				break;
			}
			else {
				return EXCEPTION_CONTINUE_SEARCH;
			}
		}

		case CppException: {
			if (wasCrashException) {
				break;
			}
			else {
				return EXCEPTION_CONTINUE_SEARCH;
			}
		}

		default:
			return EXCEPTION_CONTINUE_SEARCH;
		}

		printf("DefaultVectoredExceptionHandlerLast() call crash callback ...\n");
		handledCrashException = true;
		if (gCrashCallback) {
			gCrashCallback(pExceptionPtrs, ExceptionType::StructuredException);
			std::this_thread::sleep_for(std::chrono::milliseconds(7'000));
		}
		channelMinidump.StopChannel();
		exit(0);
	}

	LONG WINAPI DefaultUnhandledExceptionFilter(EXCEPTION_POINTERS* pExceptionPtrs) {
		if (handledCrashException) {
			printf("DefaultUnhandledExceptionFilter() crash exception already handled, ignore ... \n");
			return EXCEPTION_CONTINUE_SEARCH; // ignore next exception
		}
		
		printf("DefaultUnhandledExceptionFilter() call crash callback ...\n");
		handledCrashException = true;
		if (gCrashCallback) {
			gCrashCallback(pExceptionPtrs, ExceptionType::UnhandledException);
			std::this_thread::sleep_for(std::chrono::milliseconds(7'000));
		}
		channelMinidump.StopChannel();
		exit(0);
	}
}
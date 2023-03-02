#include "CrashHandling.h"
#include "MiniDumpMessages.h"
#include "../../../../Shared/Helpers/Helpers.h"
#include "../../../../Shared/Helpers/Channel.h"
#include "../../../../Shared/Helpers/CrashInfo.h"
#include <dbghelp.h>
#pragma comment (lib, "dbghelp.lib" )


namespace CrashHandling {
	Channel<MiniDumpMessages> channelMinidump;

	API Backtrace GetBacktrace(int SkipFrames) {
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

		wchar_t buff[1024];
		swprintf_s(buff, L"Stack hash: 0x%08lx\n", hash);

		Backtrace backtrace;
		std::wstring prevModuleFilename = L"";

		for (int i = 0; i < numFrames; ++i) {
			void* moduleBaseVoid = nullptr;
			RtlPcToFileHeader(stack[i], &moduleBaseVoid);
			auto moduleBase = (const unsigned char*)moduleBaseVoid;
			constexpr auto MODULE_BUF_SIZE = 4096U;
			wchar_t modulePath[MODULE_BUF_SIZE];
			//const wchar_t* moduleFilename = modulePath;
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
					//auto RVA = std::uint32_t((unsigned char*)stack[i] - moduleBase);
					backtrace.back().second.push_back({ moduleFilename, std::uint32_t((unsigned char*)stack[i] - moduleBase) });
				}
				//else {
				//    result += WStrPrintFormat(L"%02d:%s+0x%016llx\n", i, moduleFilename.c_str(),
				//        (std::uint64_t)stack[i]);
				//}
			}
		}
		return backtrace;
	}

	API std::wstring BacktraceToString(const Backtrace& backtrace) {
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


	API void RegisterVectorHandler(PVECTORED_EXCEPTION_HANDLER handler) {
		AddVectoredExceptionHandler(0, handler);
	}

	API void GenerateCrashReport(EXCEPTION_POINTERS* pep, const AppInfo& appInfo, std::wstring channelName) {
		auto processId = GetCurrentProcessId();
		auto threadId = GetCurrentThreadId();
		auto backtrace = GetBacktrace(0);
		try {
			channelMinidump.Open(channelName, [=](Channel<MiniDumpMessages>::ReadFunc Read, Channel<MiniDumpMessages>::WriteFunc Write) {
				auto reply = Read();
				switch (reply.type)
				{
				case MiniDumpMessages::Connect: {
					std::string strData = H::WStrToStr(appInfo.packageFolder);
					Write({ strData.begin(), strData.end() }, MiniDumpMessages::PackageFolder);

					strData = H::WStrToStr(appInfo.appCenterId);
					Write({ strData.begin(), strData.end() }, MiniDumpMessages::AppCenterId);

					strData = H::WStrToStr(appInfo.appVersion);
					Write({ strData.begin(), strData.end() }, MiniDumpMessages::AppVersion);

					strData = H::WStrToStr(appInfo.appUuid);
					Write({ strData.begin(), strData.end() }, MiniDumpMessages::AppUuid);

					strData = H::WStrToStr(appInfo.backtrace);
					Write({ strData.begin(), strData.end() }, MiniDumpMessages::Backtrace);

					strData = H::WStrToStr(appInfo.exceptionMsg);
					Write({ strData.begin(), strData.end() }, MiniDumpMessages::ExceptionMessage);

					auto crashInfo = std::make_shared<CrashInfo>();
					crashInfo->threadId = threadId;
					FillCrashInfoWithExceptionPointers(crashInfo, pep);
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
}
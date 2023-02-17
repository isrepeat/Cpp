#include "CrashHandling.h"
#include "MiniDumpMessages.h"
#include "../../../../Shared/Helpers/Helpers.h"
#include "../../../../Shared/Helpers/Channel.h"
#include "../../../../Shared/Helpers/CrashInfo.h"
#include <dbghelp.h>
#pragma comment (lib, "dbghelp.lib" )


namespace CrashHandling {
	Channel<MiniDumpMessages> channelMinidump;

	API void RegisterVectorHandler(PVECTORED_EXCEPTION_HANDLER handler) {
		AddVectoredExceptionHandler(0, handler);
	}

	API void OpenMinidumpChannel(EXCEPTION_POINTERS* pep, std::wstring packageFolder, std::wstring channelName) {

		auto hProcess = GetCurrentProcess();
		auto processId = GetCurrentProcessId();
		auto threadId = GetCurrentThreadId();

		try {
			channelMinidump.Open(channelName, [hProcess, processId, threadId, pep, packageFolder](Channel<MiniDumpMessages>::ReadFunc Read, Channel<MiniDumpMessages>::WriteFunc Write) {
				auto reply = Read();
				switch (reply.type)
				{
				case MiniDumpMessages::Connect: {
					//auto strData = std::to_string(threadId);
					//Write({ strData.begin(), strData.end() }, MiniDumpMessages::ThreadId);

					auto strData = H::WStrToStr(packageFolder);
					Write({ strData.begin(), strData.end() }, MiniDumpMessages::PackageFolder);

					CrashInfo crashInfo;
					//crashInfo.hProcess = hProcess;
					//crashInfo.processId = processId;
					crashInfo.threadId = threadId;
					FillCrashInfoWithExceptionPointers(crashInfo, pep);
					auto serializedData = SerializeCrashInfo(crashInfo);
					Write(std::move(serializedData), MiniDumpMessages::ExceptionInfo);
					break;
				}
				}

				return true;
				}, 30'000);

			channelMinidump.WaitFinishSendingMessage(MiniDumpMessages::ExceptionInfo);
			int xxx = 9;
		}
		catch (...) {
			int xxx = 9;
		}
	}
}

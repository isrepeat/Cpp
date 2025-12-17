#include "Channel.h"
#if COMPILE_FOR_DESKTOP
#include "Time.h"
#include <assert.h>
#include <chrono>
#include <mutex>

namespace HELPERS_NS {
	// TODO: rewrite this without blocking current thread
	PipeConnectionStatus WaitConnectPipe(IN HANDLE hPipe, const std::atomic<bool>& stop, int timeout) {
		LOG_FUNCTION_ENTER("WaitConnectPipe(timeout = {})", timeout);

		HELPERS_NS::Timer timer;
		std::atomic<bool> timeoutSignal = false;
		if (timeout != 0) { // 0 - infinity wait
			timer.Start(std::chrono::milliseconds{ timeout }, [&timeoutSignal] {
				timeoutSignal = true;
				});
		}

		DWORD dwBytesRead = 0;
		OVERLAPPED stOverlapped = { 0 };

		if (ConnectNamedPipe(hPipe, &stOverlapped)) {
			// Connect completed synchronously
			return PipeConnectionStatus::Connected;
		}
		else {
			auto lastErrorConnectNamedPipe = GetLastError();
			switch (lastErrorConnectNamedPipe) {
			case ERROR_IO_PENDING:
				while (true) {
					// Check break conditions in while scope (if we chek it after while may be status conflicts)
					if (stop) {
						return PipeConnectionStatus::Stopped;
					}
					if (timeoutSignal) {
						return PipeConnectionStatus::TimeoutConnection;
					}

					if (GetOverlappedResult(hPipe, &stOverlapped, &dwBytesRead, FALSE)) {
						// ConnectNamedPipe operation completed
						return PipeConnectionStatus::Connected;
					}
					else {
						// pending ...
						auto lastErrorGetOverlappedResult = GetLastError();
						if (lastErrorGetOverlappedResult != ERROR_IO_INCOMPLETE) {
							LogLastError;
							return PipeConnectionStatus::Error;
						}
					}
					Sleep(10);
				}
				break;

			case ERROR_PIPE_CONNECTED:
				return PipeConnectionStatus::Connected;

			default:
				LogLastError;
				return PipeConnectionStatus::Error;
			}
		}

		return PipeConnectionStatus::Error; // this place in the code should never be reached
	}


	PipeConnectionStatus WaitOpenPipe(OUT HANDLE& hPipe, const std::wstring& pipeName, const std::atomic<bool>& stop, int timeout) {
		LOG_FUNCTION_ENTER("WaitOpenPipe(timeout = {})", timeout);

		HELPERS_NS::Timer timer;
		std::atomic<bool> timeoutSignal = false;
		if (timeout != 0) { // 0 - infinity wait
			timer.Start(std::chrono::milliseconds{ timeout }, [&timeoutSignal] {
				timeoutSignal = true;
				});
		}

		while (!stop && !timeoutSignal) {
			hPipe = CreateFileW(pipeName.c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
			if (hPipe != INVALID_HANDLE_VALUE) {
				return PipeConnectionStatus::Connected;
			}
			Sleep(10);
		}

		if (stop) {
			return PipeConnectionStatus::Stopped;
		}
		if (timeoutSignal) {
			return PipeConnectionStatus::TimeoutConnection;
		}

		return PipeConnectionStatus::Error; // this place in the code should never be reached
	}
}
#endif
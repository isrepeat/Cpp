#include "Channel.h"
#include "Time.h"
#include <assert.h>
#include <chrono>
#include <mutex>

//#define LogDebugWithFullClassNameA(...)
//#define LogDebugWithFullClassNameW(...)
//
//#define LogWarningWithFullClassNameA(...)
//#define LogWarningWithFullClassNameW(...)
//
//#define LogErrorWithFullClassNameA(...)
//#define LogErrorWithFullClassNameW(...)
//
//#define LOG_FUNCTION_ENTER(...)
//
//#define LOG_DEBUG(...)
//#define LOG_ERROR(...)
//
//#define LOG_DEBUG_D(...)
//#define LOG_ERROR_D(...)
//
//#define LogLastError

#if NEW_CHANNEL == 2
std::exception GetLastErrorException(const std::string& msg) {
	DWORD lastError = GetLastError();
	std::string format = "%s Error %ld\n";

	std::vector<char> errMsg(2048, '\0');

	std::size_t allocatedSize = sprintf_s(errMsg.data(), errMsg.size(), format.c_str(), msg.c_str(), lastError);
	errMsg.resize(allocatedSize + 1); // Extra space for '\0';

	return std::exception(std::string{ errMsg.begin(), errMsg.end() }.c_str());
};

// TODO: rewrite this without blocking current thread
PipeConnectionStatus WaitConnectPipe(IN HANDLE hPipe, const std::atomic<bool>& stop, int timeout) {
	LOG_FUNCTION_ENTER("WaitConnectPipe(timeout = {}) ...", timeout);

	H::Timer timer;
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
	LOG_FUNCTION_ENTER("WaitOpenPipe(timeout = {}) ...", timeout);

	H::Timer timer;
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


#elif NEW_CHANNEL == 1

// TODO: rewrite this without blocking current thread
PipeConnectionStatus WaitConnectPipe(IN HANDLE hPipe, const std::atomic<bool>& stop, int timeout) {
	LOG_FUNCTION_ENTER("WaitConnectPipe(timeout = {}) ...", timeout);

	H::Timer timer;
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
	LOG_FUNCTION_ENTER("WaitOpenPipe(timeout = {}) ...", timeout);

	H::Timer timer;
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


#else

std::exception GetLastErrorException(const std::string& msg) {
	DWORD lastError = GetLastError();
	std::string format = "%s Error %ld\n";

	std::vector<char> errMsg(2048, '\0');

	std::size_t allocatedSize = sprintf_s(errMsg.data(), errMsg.size(), format.c_str(), msg.c_str(), lastError);
	errMsg.resize(allocatedSize + 1); // Extra space for '\0';

	return std::exception(std::string{ errMsg.begin(), errMsg.end() }.c_str());
};

bool WaitConnectPipe(HANDLE hPipe, const std::atomic<bool>& stop, int timeout) {
	H::Timer timer;
	std::atomic<bool> timeoutSignal = false;
	if (timeout != 0) { // 0 - infinity wait
		timer.Start(std::chrono::milliseconds{ timeout }, [&timeoutSignal] {
			timeoutSignal = true;
			});
	}

	bool result = false;
	DWORD dwBytesRead = 0;
	OVERLAPPED stOverlapped = { 0 };

	if (!ConnectNamedPipe(hPipe, &stOverlapped)) {
		switch (GetLastError())
		{
		case ERROR_IO_PENDING: {
			while (!stop && !timeoutSignal) {
				if (GetOverlappedResult(hPipe, &stOverlapped, &dwBytesRead, FALSE)) {
					// Connect operation completed ...
					result = true;
					break;
				}
				else {
					// pending ...
					if (GetLastError() != ERROR_IO_INCOMPLETE) {
						////throw GetLastErrorException("ConnectNamedPipeAsync IO incomplete! ");
						//std::vector<char> err(256, '\0');
						//sprintf(err.data(), "%s Error %ld\n", "not pending error ...", GetLastError());
						//Log(std::string{ err.begin(), err.end() });
						return false;
					}
				}

				Sleep(10);
			}
			break;
		}
		case ERROR_PIPE_CONNECTED: {
			return true;
		}

		default: {
			////throw GetLastErrorException("ConnectNamedPipeAsync: "); // Connect some error ...
			//std::vector<char> err(256, '\0');
			//sprintf(err.data(), "%s Error %ld\n", "Connect some error ...", GetLastError());
			//Log(std::string{ err.begin(), err.end() });
			return false;
		}
		}

	}
	else {
		// Connect completed synchronously ...
		result = true;
	}

	return result;
}

HANDLE WaitOpenPipe(const std::wstring& pipeName, const std::atomic<bool>& stop, int timeout) {
	H::Timer timer;
	std::atomic<bool> timeoutSignal = false;
	if (timeout != 0) { // 0 - infinity wait
		timer.Start(std::chrono::milliseconds{ timeout }, [&timeoutSignal] {
			timeoutSignal = true;
			});
	}

	HANDLE hPipe = INVALID_HANDLE_VALUE;
	while (!stop && !timeoutSignal) {
		hPipe = CreateFileW(pipeName.c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
		if (hPipe != INVALID_HANDLE_VALUE)
			break;

		Sleep(10);
	}

	return hPipe;
}
#endif
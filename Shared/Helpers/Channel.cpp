#include "Time.h"
#include "Channel.h"
#include <assert.h>
#include <chrono>
#include <mutex>


std::exception GetLastErrorException(const std::string& msg) {
	std::vector<char> err(256, '\0');
	sprintf_s(err.data(), err.size(), "%s Error %ld\n", msg, GetLastError());
	return std::exception(std::string{ err.begin(), err.end() }.c_str());
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
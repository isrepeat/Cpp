#include "Time.h"
#include "Channel.h"
#include <assert.h>
#include <chrono>
#include <mutex>


//std::wstring GetTokenUserSid(HANDLE hToken) {
//	PTOKEN_USER ptu = NULL;
//	DWORD dwSize = 0;
//	if (!GetTokenInformation(hToken, TokenUser, NULL, 0, &dwSize) && GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
//		return L"";
//	}
//	if ((ptu = (PTOKEN_USER)LocalAlloc(LPTR, dwSize)) != NULL)
//	{
//		LPTSTR StringSid = NULL;
//		if (!GetTokenInformation(hToken, TokenUser, ptu, dwSize, &dwSize))
//		{
//			LocalFree((HLOCAL)ptu);
//			return L"";
//		}
//		if (ConvertSidToStringSid(ptu->User.Sid, &StringSid))
//		{
//			auto strToken = std::wstring{ StringSid };
//			//_tprintf(_T("%s\n"), StringSid);
//			LocalFree((HLOCAL)StringSid);
//			LocalFree((HLOCAL)ptu);
//			return strToken;
//		}
//		LocalFree((HLOCAL)ptu);
//	}
//	return L"";
//}

std::wstring GetTokenUserSid(HANDLE hToken) {
	PTOKEN_APPCONTAINER_INFORMATION ptu = NULL;
	DWORD dwSize = 0;
	if (!GetTokenInformation(hToken, TokenAppContainerSid, NULL, 0, &dwSize) && GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
		return L"";
	}
	if ((ptu = (PTOKEN_APPCONTAINER_INFORMATION)LocalAlloc(LPTR, dwSize)) != NULL)
	{
		LPTSTR StringSid = NULL;
		if (!GetTokenInformation(hToken, TokenAppContainerSid, ptu, dwSize, &dwSize))
		{
			LocalFree((HLOCAL)ptu);
			return L"";
		}
		if (ConvertSidToStringSid(ptu->TokenAppContainer, &StringSid))
		{
			auto strToken = std::wstring{ StringSid };
			//_tprintf(_T("%s\n"), StringSid);
			LocalFree((HLOCAL)StringSid);
			LocalFree((HLOCAL)ptu);
			return strToken;
		}
		LocalFree((HLOCAL)ptu);
	}
	return L"";
}




//LocalPtr<TOKEN_APPCONTAINER_INFORMATION> GetTokenAppContainerInfo(HANDLE hToken) {
//
//	DWORD dwSize = 0;
//	if (!GetTokenInformation(hToken, ::TokenAppContainerSid, NULL, 0, &dwSize) && GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
//		return nullptr;
//	}
//	
//	return LocalPtr<TOKEN_APPCONTAINER_INFORMATION>(LocalAlloc(LPTR, dwSize));
//	//return LocalPtr<PTOKEN_APPCONTAINER_INFORMATION>(LocalAlloc(LPTR, dwSize));
//
//	//if (pAppContainerInfo)
//	//{
//	//	if (!GetTokenInformation(hToken, ::TokenAppContainerSid, pTokenData.get(), dwSize, &dwSize))
//	//	{
//	//		//LocalFree((HLOCAL)pTokenData);
//	//		throw std::exception("fail");
//	//	}
//
//	////	LPWSTR StringSid = NULL;
//
//	////	pTokenData.get()
//
//	////	//if (!ConvertSidToStringSid(pTokenData->TokenAppContainer, &szSid)
//
//	////	return TokenAppContainerInfo{ pTokenData };
//	////}
//
//	//return nullptr;
//}


//TokenInfo GetTokenInfo(HANDLE hToken) {
//	
//
//	if (auto appContainerTokenInfo = GetTokenAppContainerInfo(hToken)) {
//		if (auto sessionId = GetTokenSessingId(hToken)) {
//
//		}
//	}
//
//
//
//	PTOKEN_APPCONTAINER_INFORMATION appConainerInfo;
//
//	ULONG err = BOOL_TO_ERROR(GetTokenInformation(hToken, ::TokenAppContainerSid, buf, cb, &rcb));
//	if (err == NOERROR) {
//		ULONG SessionId;
//
//		err = BOOL_TO_ERROR(GetTokenInformation(hToken, ::TokenSessionId, &SessionId, sizeof(SessionId), &rcb));
//		if (err == NOERROR) {
//			PWSTR szSid;
//
//			err = BOOL_TO_ERROR(ConvertSidToStringSid(AppConainer->TokenAppContainer, &szSid));
//			if (err == NOERROR) {
//				static const WCHAR fmt[] = L"\\\\?\\pipe\\Sessions\\%d\\AppContainerNamedObjects\\%s\\%s";
//
//				rcb = (1 + _scwprintf(fmt, SessionId, szSid, pipeName.c_str())) * sizeof(WCHAR);
//
//				if (cb < rcb) {
//					cb = RtlPointerToOffset(buf = alloca(rcb - cb), stack);
//				}
//
//				_swprintf(sz, fmt, SessionId, szSid, pipeName.c_str());
//}

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



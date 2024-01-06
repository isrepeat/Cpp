#include <Windows.h>
#include <iostream>
#include <cassert>
#include <string>
#include <vector>

#define BUFSIZE	 8192

const std::string FindFormatComplete = "Format complete.";
const std::string FormatComplete = "are available.";
const std::string VolumeLabel = "Volume label (32 characters, ENTER for none)? ";
const std::string CompletedProgress = "percent completed";
const std::string FindReadyToParse = "ReadyToParse";

bool IsEndOfLine(char c) {
	return c == '\r' || c == '\n';
}

bool Contains(const std::string& a, const std::string& b) {
	return a.find(b) != std::string::npos;
}


std::string GetLastLine(const std::string& buf) {
	if (buf.empty()) {
		return std::string();
	}

	size_t idx = buf.size() - 1;

	while (idx > 0 && IsEndOfLine(buf[idx])) {
		idx--;
	}

	auto strEnd = idx + 1;

	while (idx > 0 && !IsEndOfLine(buf[idx])) {
		idx--;
	}

	auto strStart = idx;

	if (IsEndOfLine(buf[strStart])) {
		strStart++;
		assert(!IsEndOfLine(buf[strStart]));
	}

	return buf.substr(strStart, strEnd - strStart);
}



BOOL _ReadPipe(HANDLE pipe, std::string& buf) {
	unsigned long lpBytesRead, lpBytesLeft, readed;

	std::vector<char> tmp(BUFSIZE);
	lpBytesRead = 0;
	lpBytesLeft = 0;

	while (lpBytesRead == 0 && PeekNamedPipe(pipe, tmp.data(), static_cast<DWORD>(tmp.size()), &readed, &lpBytesRead, &lpBytesLeft)) {
		if (readed == 0)
			return FALSE;
	}

	buf = std::string(tmp.begin(), tmp.begin() + readed);
	//OutputDebugStringA(buf.c_str());

	while (!buf.empty() && IsEndOfLine(buf.back())) {
		buf.pop_back();
	}

	return TRUE;
}

BOOL ReadFromPipe(HANDLE pipe, std::string& buf) {
	if (!_ReadPipe(pipe, buf)) {
		return FALSE;
	}

	if (buf.rfind(FindFormatComplete) != std::string::npos) {
		buf = FormatComplete;
		return TRUE;
	}

	buf = GetLastLine(buf);

	if (buf == VolumeLabel) {
		return TRUE;
	}

	int progress_size = std::string(CompletedProgress).size();
	if (Contains(buf, CompletedProgress)) {
		size_t end_index = buf.rfind(CompletedProgress);
		auto start_index = end_index;

		while (true) {
			if (start_index == 0)
				break;
			else if (buf.at(start_index) == '\r')
				break;
			else
				start_index--;
		}

		if (start_index == 0)
			buf = FindReadyToParse + buf.substr(start_index, end_index - start_index - 1);
		else
			buf = FindReadyToParse + buf.substr(start_index + 1, end_index - start_index - 2);
	}

	return TRUE;
}

std::string GetCMDInfo(HANDLE pipe) {
	std::string buf;
	std::string prev_buff;
	while (true) {
		//Sleep(100);
		BOOL result1 = ReadFromPipe(pipe, buf);
		if (result1 == TRUE && prev_buff == buf)
			break;
		else
			prev_buff = buf;
	}
	return buf;
}

bool WriteStringToFile(HANDLE hFile, const std::string& buffer) {
	return ::WriteFile(hFile, buffer.c_str(), buffer.size(), NULL, NULL);
}





int main() {
	//TCHAR cmdLine[128] = { 0 };
	//_tcscpy_s(cmdLine, _T("C:\\Windows\\system32\\cmd.exe"));
	
	BOOL b = S_OK;
	DWORD dwRead, dwWritten;

	//std::wstring cmdLine = L"/k \"format h: \fs & exit\"";
	std::wstring cmdLine = L"C:\\Windows\\system32\\cmd.exe";


	HANDLE hReadPipe1, hWritePipe1, hReadPipe2, hWritePipe2;
	SECURITY_ATTRIBUTES sa = { sizeof(SECURITY_ATTRIBUTES), NULL, TRUE };

	HANDLE hStdRead = NULL, hStdWrite = NULL;
	b = CreatePipe(&hStdRead, &hStdWrite, &sa, 0);

	//if (!CreatePipe(&hReadPipe1, &hWritePipe1, &sa, BUFSIZE))
	//	return 0;

	//if (!CreatePipe(&hReadPipe2, &hWritePipe2, &sa, BUFSIZE))
	//	return 0;

	//b = SetHandleInformation(hStdWrite, HANDLE_FLAG_INHERIT, 0);

	// Ensure the read handle to the pipe for STDOUT is not inherited.
	if (!SetHandleInformation(hStdRead, HANDLE_FLAG_INHERIT, 0)) {
		printf("Stdout SetHandleInformation");
	}

	STARTUPINFO si = {};
	GetStartupInfoW(&si);

	si.cb = sizeof(si);
	si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
	si.wShowWindow = SW_SHOW;

	si.hStdError = hStdWrite;
	si.hStdOutput = hStdWrite;
	si.hStdInput = hStdRead;

	//si.hStdInput = hReadPipe2;
	//si.hStdOutput = si.hStdError = hWritePipe1;
	

	PROCESS_INFORMATION pi = {};
	//if (CreateProcessW(L"C:\\Windows\\system32\\cmd.exe", (LPWSTR)cmdLine.c_str(), NULL, NULL, TRUE, NULL, NULL, NULL, &si, &pi))
	//if (CreateProcessW(L"C:\\Windows\\system32\\cmd.exe", NULL, NULL, NULL, FALSE, CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi))
	if (CreateProcessW(NULL, (LPWSTR)cmdLine.c_str(), NULL, NULL, FALSE, CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi))
	{
		//WriteConsoleA(hStdRead, "dir", 3, &dwWritten, NULL);
		//CloseHandle(g_hChildStd_OUT_Rd);

		//Sleep(1000);

		//auto buff = GetCMDInfo(hReadPipe1);
		//
		WriteStringToFile(hStdRead, "format h: \fs & exit");

		//Sleep(1000);

		//buff = GetCMDInfo(hReadPipe1);

		//Sleep(1000);

		//buff = GetCMDInfo(hReadPipe1);

		//Sleep(1000);

		//buff = GetCMDInfo(hReadPipe1);

		//Sleep(1000);



		//CloseHandle(hStdRead);
		//hStdRead = NULL;

		//Sleep(1000); // Give command prompt window time to initialize

		//// Attach to command prompt console and change code page before running batch file
		//BOOL b = AttachConsole(pi.dwProcessId);

		//b = SetConsoleCP(CP_UTF8);

		//b = SetConsoleOutputCP(CP_UTF8);

		//b = FreeConsole();

		////char pipecmd[] = "tst.bat\r\nexit\r\n";
		//std::string testCommand = "format h: \fs\r\n";
		//DWORD dwWritten = 0;

		//// Pipe batch file commands to console
		//b = WriteFile(hStdWrite, testCommand.c_str(), testCommand.size(), &dwWritten, NULL);
		//Sleep(10'000);

		//CloseHandle(hStdWrite);
		//hStdWrite = NULL;
		//CloseHandle(pi.hThread);
		//CloseHandle(pi.hProcess);
	}
	else
	{
		//CloseHandle(hStdRead);
		//CloseHandle(hStdWrite);
	}

	Sleep(10'000);

	return 0;
}



//
//#include <iostream>
//#include <windows.h>
//
//
//#define deffBuffSize 1024
//
//
//
//
//int funSC() {
//
//    HANDLE writeToCL, readFromCL, writeToProcess, readFromProcess;
//    char lpCmdLine[] = "cmd.exe /k ipconfig";
//    STARTUPINFOA siA;
//    PROCESS_INFORMATION piApp;
//    SECURITY_ATTRIBUTES secAttr;
//    ZeroMemory(&secAttr, sizeof(SECURITY_ATTRIBUTES));
//    ZeroMemory(&siA, sizeof(STARTUPINFOA));
//
//    secAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
//    secAttr.lpSecurityDescriptor = NULL;
//    secAttr.bInheritHandle = TRUE;
//
//    if (CreatePipe(&readFromCL, &writeToProcess, &secAttr, 0) == 0) {
//        std::cout << "Create pipe error :: " << GetLastError() << std::endl;
//        return 1;
//    }
//
//    if (!SetHandleInformation(readFromCL, HANDLE_FLAG_INHERIT, 0)) {
//        std::cout << "SetHandleInformation error :: " << GetLastError() << std::endl;
//    }
//    if (!SetHandleInformation(writeToProcess, HANDLE_FLAG_INHERIT, 0)) {
//        std::cout << "SetHandleInformation error :: " << GetLastError() << std::endl;
//    }
//
//    if (CreatePipe(&readFromProcess, &writeToCL, &secAttr, 0) == 0) {
//        std::cout << "Create pipe error :: " << GetLastError() << std::endl;
//        return 1;
//    }
//    if (!SetHandleInformation(readFromProcess, HANDLE_FLAG_INHERIT, 0)) {
//        std::cout << "SetHandleInformation error :: " << GetLastError() << std::endl;
//    }
//    if (!SetHandleInformation(writeToCL, HANDLE_FLAG_INHERIT, 0)) {
//        std::cout << "SetHandleInformation error :: " << GetLastError() << std::endl;
//    }
//
//
//
//    siA.cb = sizeof(STARTUPINFOA);
//    siA.hStdInput = readFromProcess;
//    siA.hStdOutput = writeToProcess;
//    siA.hStdError = writeToProcess;
//    siA.dwFlags = STARTF_USESTDHANDLES;
//
//
//    if (CreateProcessA(NULL, lpCmdLine, NULL, NULL, TRUE, CREATE_NEW_CONSOLE, NULL, NULL, &siA, &piApp) == 0) {
//        std::cout << "CreateProcessA error :: " << GetLastError() << std::endl;
//        return 1;
//    }
//    else {
//        CloseHandle(readFromProcess);
//        CloseHandle(writeToProcess);
//    }
//
//    DWORD dRead = 0;
//    char chBuff[deffBuffSize];
//    bool bSuccess = FALSE;
//    memset(chBuff, '\0', deffBuffSize);
//    std::string outStd;
//
//
//    for (;;) {
//        memset(chBuff, '\0', deffBuffSize);
//        bSuccess = ReadFile(readFromCL, chBuff, deffBuffSize, &dRead, NULL);
//        if (!bSuccess || dRead == 0) {
//            break;
//        }
//        else {
//
//            std::cout << chBuff;
//        }
//    }
//
//    //bSuccess = ReadFile(readFromCL, chBuff, deffBuffSize, &dRead, NULL);
//
//    std::cout << "bSuccess: " << bSuccess << std::endl;
//    std::cout << "GetLastError: " << GetLastError() << std::endl;
//
//
//    std::cout << "Message: " << chBuff;
//    //CloseHandle(writeToCL);
//    //CloseHandle(readFromCL);
//    CloseHandle(piApp.hProcess);
//    CloseHandle(piApp.hThread);
//    return 0;
//}
//
//int main() {
//    int result = 0;
//    result = funSC();
//    system("pause");
//    return result;
//}
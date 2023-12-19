#pragma once
#include "Platform.h"
#include <P7_Client.h>
#include <P7_Trace.h>
#include <string>


#ifdef __WINDOWS_OS__
namespace {
	std::wstring StringToWString(const std::string& s) {
		int slength = s.length() + 1;
		auto resultLength = MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, 0, 0);
		std::wstring resultString;
		resultString.resize(resultLength);
		MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, resultString.data(), resultLength);
		return resultString;
	}
}

// ##__VA_ARGS__ - remove ',' if nothing args was not passed
// NOTE: use Sleep(1) to avoid access violation temporary std::string
#define WriteInfo(message, ...) Logger::GetInstance().trace->P7_INFO(Logger::GetInstance().hModule, StringToWString(message).c_str(), ##__VA_ARGS__); P7_Flush(); P7_Client_Flush(Logger::GetInstance().logger); Sleep(1);
#define WriteError(message, ...) Logger::GetInstance().trace->P7_ERROR(Logger::GetInstance().hModule, StringToWString(message).c_str(), ##__VA_ARGS__); P7_Flush();P7_Client_Flush(Logger::GetInstance().logger); Sleep(1);
#define WriteDebug(message, ...) Logger::GetInstance().trace->P7_DEBUG(Logger::GetInstance().hModule, StringToWString(message).c_str(), ##__VA_ARGS__); P7_Flush();P7_Client_Flush(Logger::GetInstance().logger); Sleep(1);
#define WriteTrace(message, ...) Logger::GetInstance().trace->P7_TRACE(Logger::GetInstance().hModule, StringToWString(message).c_str(), ##__VA_ARGS__); P7_Flush();P7_Client_Flush(Logger::GetInstance().logger); Sleep(1);

#else
// NOTE: use const char* strings to avoid access violation
#define WriteInfo(message, ...) Logger::GetInstance().trace->P7_INFO(Logger::GetInstance().hModule, message, ##__VA_ARGS__); P7_Flush(); P7_Client_Flush(Logger::GetInstance().logger);
#define WriteError(message, ...) Logger::GetInstance().trace->P7_ERROR(Logger::GetInstance().hModule, message, ##__VA_ARGS__); P7_Flush();P7_Client_Flush(Logger::GetInstance().logger);
#define WriteDebug(message, ...) Logger::GetInstance().trace->P7_DEBUG(Logger::GetInstance().hModule, message, ##__VA_ARGS__); P7_Flush();P7_Client_Flush(Logger::GetInstance().logger);
#define WriteTrace(message, ...) Logger::GetInstance().trace->P7_TRACE(Logger::GetInstance().hModule, message, ##__VA_ARGS__); P7_Flush();P7_Client_Flush(Logger::GetInstance().logger);

#endif

class Logger
{
public:
	static Logger& GetInstance();

	~Logger();
private:
	Logger();
public:
	IP7_Client* logger;
	IP7_Trace* trace;
	IP7_Trace::hModule hModule;
};
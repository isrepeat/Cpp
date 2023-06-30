#pragma once
#include <iostream>

#ifdef __MAKE_DLL__
#define API __declspec(dllexport)
#else
#define API // for static
//#define API __declspec(dllimport) // for dll
#endif


namespace google {
	class LogMessage;
}

namespace LoggerCpp {
	enum class LogSeverity {
		_INFO,
		_WARNING,
		_ERROR, // add _ to avoid conficts with ERROR from Windows.h
		_FATAL
	};

	class LogMessage {
	public:
		LogMessage(LogSeverity severity, const char* file, const char* function, int line);
		~LogMessage();

		std::ostream& Stream();

	private:
		google::LogMessage* instance;
		const char* functionName;
	};
	
	API void SetLogDestination(LogSeverity severity, const std::wstring& filename, bool addTimestamp = true);
	API void SetLogDestination(const std::wstring& filename, bool addTimestamp = true);
	API void SetLogFilenameExtension(const std::wstring& ext);
	API void InitLogger(const char* argv0);
}

#define LOG_INFO  LoggerCpp::LogMessage(LoggerCpp::LogSeverity::_INFO, __FILE__, __FUNCTION__, __LINE__).Stream()
#define LOG_ERROR LoggerCpp::LogMessage(LoggerCpp::LogSeverity::_ERROR, __FILE__, __FUNCTION__, __LINE__).Stream()
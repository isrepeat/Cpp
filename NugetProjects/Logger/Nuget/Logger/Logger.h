#pragma once
#include <iostream>
#include <memory>

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
	enum class LogSaverity {
		_INFO,
		_WARNING,
		_ERROR, // add _ to avoid conficts with ERROR from Windows.h
		_FATAL
	};

	class LogMessage {
	public:
		//_LogMessage(LogSaverity saverity, const char* file, const char* function, int line);
		LogMessage(LogSaverity saverity, const char* file, int line);
		~LogMessage();

		std::ostream& Stream();

	private:
		google::LogMessage* instance;
	};

	
	API void SetLogDestination(LogSaverity saverity, const std::wstring& filename, bool addTimestamp = true);
	API void SetLogFilenameExtension(const std::wstring& ext);
	API void InitLogger(const char* argv0);
}

#define LOG_INFO LoggerCpp::LogMessage(LoggerCpp::LogSaverity::_INFO, __FILE__, __LINE__).Stream() << __FUNCTION__ "} "
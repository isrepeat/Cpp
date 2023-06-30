#pragma once
#include <atomic>
#include <string>
#include <iostream>
#include <functional>

#ifdef __STATIC_LIBRARY__
	#define API
#else
#ifdef __DYNAMIC_LIBRARY__
	#define API __declspec(dllexport)
#else
	#define API __declspec(dllimport)
#endif
#endif


namespace google {
	class LogMessage;
}

namespace LoggerCpp {
	namespace FLAGS {
		extern std::atomic<bool> log_without_prefix;
	}

	enum class LogSeverity {
		_INFO,
		_WARNING,
		_ERROR, // add _ to avoid conficts with ERROR from Windows.h
		_FATAL
	};

	struct API LogMessageHeader {
		const std::string lineFormat;
		const std::string additoinalInfo;
	};

	struct API LogMessageTime {
		const int year;
		const int month;
		const int day;
		const int hour;
		const int min;
		const int sec;
		const int usec;
	};

	struct API LogMessageInfo {
		LogMessageInfo(const char* const severity, const char* const filename,
			const char* const function, const int& line_number, const int& thread_id, const LogMessageTime& time);

		const char* const severity;
		const char* const filename;
		const char* const function;
		const int& line_number;
		const int& thread_id;
		const LogMessageTime& time;
	};


	class API LogMessage {
	public:
		LogMessage(LogSeverity severity, const char* file, const char* function, int line);
		~LogMessage();

		std::ostream& Stream();

	private:
		google::LogMessage* instance;
	};


	API void SetLogDestination(LogSeverity severity, const std::wstring& filename, bool addTimestamp = true);
	API void SetLogDestination(const std::wstring& filename, bool addTimestamp = true);
	API void SetLogFilenameExtension(const std::wstring& ext);

	API void InitLogger(const char* argv0, std::function<void(std::ostream&, const LogMessageInfo&)> prefixCallback = nullptr, const LogMessageHeader& logHeader = {});
}

#define LOG_INFO  LoggerCpp::LogMessage(LoggerCpp::LogSeverity::_INFO, __FILE__, __FUNCTION__, __LINE__).Stream()
#define LOG_ERROR LoggerCpp::LogMessage(LoggerCpp::LogSeverity::_ERROR, __FILE__, __FUNCTION__, __LINE__).Stream()
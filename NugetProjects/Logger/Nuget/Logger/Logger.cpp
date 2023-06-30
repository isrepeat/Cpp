#include <glog/logging.h>
#include <iostream>
#include <iomanip>
#include <mutex>
#include "Logger.h"
#include "String.h"


namespace LoggerCpp {
	namespace FLAGS {
		std::atomic<bool> log_without_prefix = false;
	}

	// static global definitions
	namespace {
		struct AdditionalLogContext {
			const char* functionName;
		};

		struct SharedLogMessageContext {
			std::mutex mx;
			AdditionalLogContext* context = nullptr;
		};
		SharedLogMessageContext* const sharedLogMessageContext = new SharedLogMessageContext; // define as global pointer to avoid destruction order problem on exit program


		std::function<void(std::ostream&, const LogMessageInfo&)> customPrefixCallback = nullptr;

		void PrefixCallback(std::ostream& s, const google::LogMessageInfo& l, void*) {
			if (FLAGS::log_without_prefix) {
				// TOOD: find out how ignore next space symbol that glog lib place automatically
				return;
			}

			auto functionName = sharedLogMessageContext->context->functionName;

			if (customPrefixCallback) {
				customPrefixCallback(s, LogMessageInfo{
						l.severity,
						l.filename,
						functionName,
						l.line_number,
						l.thread_id,
						LogMessageTime{ l.time.year(), l.time.month(), l.time.day(), l.time.year(), l.time.month(), l.time.day() }
					});
			}
			else {
				s << std::left
					<< std::setfill(' ')
					<< std::setw(5) << l.severity
					<< " "
					<< std::setw(6) << l.thread_id
					<< " ["
					<< std::right
					<< std::setfill('0')
					<< std::setw(2) << l.time.day() << '.'
					<< std::setw(2) << 1 + l.time.month() << '.'
					<< std::setw(4) << 1900 + l.time.year()
					<< " "
					<< std::setw(2) << l.time.hour() << ':'
					<< std::setw(2) << l.time.min() << ':'
					<< std::setw(2) << l.time.sec() << "."
					<< std::setw(2) << l.time.usec()
					<< std::setfill(' ')
					<< "] "
					<< "{" << l.filename << ':' << l.line_number << " " << functionName << "}";
			}
		}
	} // static global definitions


	LogMessageInfo::LogMessageInfo(const char* const severity, const char* const filename,
		const char* const function, const int& line_number, const int& thread_id, const LogMessageTime& time)
		: severity{ severity }
		, filename{ filename }
		, function{ function }
		, line_number{ line_number }
		, thread_id{ thread_id }
		, time{ time }
	{}

	LogMessage::LogMessage(LogSeverity severity, const char* file, const char* function, int line)  {
		std::unique_lock<std::mutex> lk{ sharedLogMessageContext->mx };
		auto additionalLogContext = AdditionalLogContext{ function };
		sharedLogMessageContext->context = &additionalLogContext;
		instance = new google::LogMessage(file, line, static_cast<int>(severity)); // PrefixCallback called here internal
	}

	LogMessage::~LogMessage() {	
		delete instance;
	}

	std::ostream& LogMessage::Stream() {
		return instance->stream();
	}


	API void SetLogDestination(LogSeverity severity, const std::wstring& filename, bool addTimestamp) {
		std::wstring finalFilename = filename;
		if (addTimestamp) {
			finalFilename += L"_[";
			FLAGS_timestamp_in_logfile_name = true;
		}
		else {
			FLAGS_timestamp_in_logfile_name = false;
		}
		google::SetLogDestination(static_cast<int>(severity), WStrToStr(finalFilename).c_str());
	}

	API void SetLogDestination(const std::wstring& filename, bool addTimestamp) {
		SetLogDestination(LogSeverity::_INFO, filename, addTimestamp);
	}

	API void SetLogFilenameExtension(const std::wstring& ext) {
		std::wstring finalExtension = FLAGS_timestamp_in_logfile_name ? L"]"+ext : ext;
		google::SetLogFilenameExtension(WStrToStr(finalExtension).c_str());
	}


	API void InitLogger(const char* argv0, std::function<void(std::ostream&, const LogMessageInfo&)> prefixCallback, const LogMessageHeader& logHeader) {
		customPrefixCallback = prefixCallback;
		google::InitGoogleLogging(argv0, &PrefixCallback);

		FLAGS_log_file_header = false;
		FLAGS_logbuflevel = -1; // this mean that INFO logs will flush every log

		FLAGS::log_without_prefix = true;
		LOG_INFO << "----------------------------------------------------------------------------------------------------";
		if (!logHeader.additoinalInfo.empty()) {
			LOG_INFO << logHeader.additoinalInfo;
		}
		if (!logHeader.lineFormat.empty()) {
			LOG_INFO << logHeader.lineFormat;
		}
		else {
			LOG_INFO << "Log line format: severity thread_id [dd.mm.yyy hh:mm:ss.uuuuuu] {filename:line function} log_message";
		}
		LOG_INFO << "----------------------------------------------------------------------------------------------------";
		LOG_INFO << "\n";
		FLAGS::log_without_prefix = false;
	}
}

#define __MAKE_DLL__
#include <glog/logging.h>
#include <iostream>
#include <iomanip>
#include "Logger.h"
#include "String.h"

// Don't forget increase version according to nuget
#define VERSION "1.0.0"

namespace {
	bool FLAG_log_without_prefix = false;

	void CustomPrefix(std::ostream& s, const google::LogMessageInfo& l, void*) {
		if (FLAG_log_without_prefix) {
			// TOOD: find out how ignore next space symbol that glog lib place automatically
			return;
		}

		s << std::left
			<< std::setfill(' ')
			<< std::setw(5) << l.severity
			<< " "
			<< std::setw(6) << l.thread_id
			<< " ["
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
			<< "{" << l.filename << ':' << l.line_number << " "; // after this following __FUNCTION__ as designed with trailing '}'
	}
}

namespace LoggerCpp {
	LogMessage::LogMessage(LogSeverity severity, const char* file, const char* function, int line)
		: instance{ new google::LogMessage(file, line, static_cast<int>(severity)) }
		, functionName{ function }
	{}

	LogMessage::~LogMessage() {
		delete instance;
	}

	std::ostream& LogMessage::Stream() {
		if (FLAG_log_without_prefix)
			return instance->stream();
		else
			return instance->stream() << functionName << "} ";
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

	API void InitLogger(const char* argv0) {
		google::InitGoogleLogging(argv0, &CustomPrefix);
		FLAGS_log_file_header = false;
		FLAGS_logbuflevel = -1; // this mean that INFO logs will flush every log

		FLAG_log_without_prefix = true;
		LOG_INFO << "Logger version = " << VERSION;
		LOG_INFO << "Log line format: severity thread_id [dd.mm.yyy hh:mm:ss.uuuuuu] {filename:line	function} log_message ...\n\n";
		FLAG_log_without_prefix = false;
	}
}

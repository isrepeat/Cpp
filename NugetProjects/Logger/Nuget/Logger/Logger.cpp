//#define __MAKE_DLL__
#include <glog/logging.h>
#include <iostream>
#include <iomanip>
#include "Logger.h"
#include "String.h"

namespace {
	void CustomPrefix(std::ostream& s, const google::LogMessageInfo& l, void*) {
		using namespace std;
		s << setfill(' ') 
			<< setw(4) << l.severity 
			<< " "
			<< setw(6) << l.thread_id
			<< " [" 
			<< setfill('0')
			<< setw(2) << l.time.day() << '.'
			<< setw(2) << 1 + l.time.month() << '.'
			<< setw(4) << 1900 + l.time.year()
			<< " "
			<< setw(2) << l.time.hour() << ':'
			<< setw(2) << l.time.min() << ':'
			<< setw(2) << l.time.sec() << "."
			<< setw(2) << l.time.usec()
			<< setfill(' ')
			<< "] "
			<< "{" << l.filename << ':' << l.line_number << " "; // after this following __FUNCTION__ as designed with trailing '}'
	}
}

namespace LoggerCpp {
	LogMessage::LogMessage(LogSaverity saverity, const char* file, int line)
		: instance{ new google::LogMessage(file, line, static_cast<int>(saverity)) }
	{}

	LogMessage::~LogMessage() {
		delete instance;
	}

	std::ostream& LogMessage::Stream() {
		return instance->stream();
	}


	API void SetLogDestination(LogSaverity saverity, const std::wstring& filename, bool addTimestamp) {
		std::wstring finalFilename = filename;
		if (addTimestamp) {
			finalFilename += L"_[";
			FLAGS_timestamp_in_logfile_name = true;
		}
		else {
			FLAGS_timestamp_in_logfile_name = false;
		}
		google::SetLogDestination(static_cast<int>(saverity), WStrToStr(finalFilename).c_str());
	}

	API void SetLogFilenameExtension(const std::wstring& ext) {
		std::wstring finalExtension = FLAGS_timestamp_in_logfile_name ? L"]"+ext : ext;
		google::SetLogFilenameExtension(WStrToStr(finalExtension).c_str());
	}

	API void InitLogger(const char* argv0) {
		google::InitGoogleLogging(argv0, &CustomPrefix);
		FLAGS_log_file_header = false;
		FLAGS_logbuflevel = -1; // this mean that INFO logs will flush every log
	}
}

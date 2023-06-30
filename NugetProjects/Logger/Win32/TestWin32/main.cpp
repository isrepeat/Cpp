#include <Windows.h>
#include <iomanip>
#include <thread>

//#include "../../Nuget/Logger/Logger.h"
#include <Logger/Logger.h>


void MyPrefixCallback(std::ostream& s, const LoggerCpp::LogMessageInfo& lgInfo) {
	s << std::left
		<< std::setfill(' ')
		<< std::setw(5) << lgInfo.severity
		<< " "
		<< std::setw(6) << lgInfo.thread_id
		<< " ["
		<< std::right
		<< std::setfill('0')
		<< std::setw(2) << lgInfo.time.day << '.'
		<< std::setw(2) << 1 + lgInfo.time.month << '.'
		<< std::setw(4) << 1900 + lgInfo.time.year
		<< " "
		<< std::setw(2) << lgInfo.time.hour << ':'
		<< std::setw(2) << lgInfo.time.min << ':'
		<< std::setw(2) << lgInfo.time.sec << "."
		<< std::setw(2) << lgInfo.time.usec
		<< std::setfill(' ')
		<< "] "
		<< "(" << lgInfo.filename << ':' << lgInfo.line_number << " " << lgInfo.function << ")";
}

int main(int /*argc*/, char** argv) {
	LoggerCpp::SetLogDestination(L"D:\\WORK\\TEST\\Loggers\\LoggerCpp\\Logger_1.0.0");
	LoggerCpp::SetLogFilenameExtension(L".txt");

	LoggerCpp::InitLogger(argv[0], &MyPrefixCallback, {"some line format ...", "AppVersion: 1.1.0.1 \n System Arch: x64"});


	LOG_INFO << "one = " << 1234;
	LOG_ERROR << "error = " << 5678;
	LOG_INFO << "Last log";

	return 0;
}
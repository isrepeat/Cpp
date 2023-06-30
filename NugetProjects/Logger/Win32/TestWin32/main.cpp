#include <Windows.h>
#include <thread>

#include "../../Nuget/Logger/Logger.h"


int main(int /*argc*/, char** argv) {
	LOG_INFO << "before 1";
	LoggerCpp::SetLogDestination(LoggerCpp::LogSaverity::_INFO, L"D:\\WORK\\TEST\\Loggers\\Glog\\Logger_1.0.0");
	LOG_INFO << "before 2";
	LoggerCpp::SetLogFilenameExtension(L".txt");
	LOG_INFO << "before 3";
	LoggerCpp::InitLogger(argv[0]);


	LOG_INFO << "one = " << 1488;
	LOG_INFO << "Last log";

	return 0;
}
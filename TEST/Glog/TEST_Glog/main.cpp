//#define GLOG_NO_ABBREVIATED_SEVERITIES
//#define WIN32_LEAN_AND_MEAN
//#include <windows.h>
#include <glog/logging.h>
#include <glog/raw_logging.h>
#include "Helpers.h"
#include <iostream>
#include <iomanip>
#pragma comment(lib, "glog.lib")


#define TEST_FOLDER "D:\\WORK\\TEST\\Loggers\\Glog"

void CustomPrefix(std::ostream& s, const google::LogMessageInfo& l, void*) {
    using namespace std;

    s << "[" << l.thread_id << setfill('0') << "] "
        << l.severity << " "
        << setw(4) << 1900 + l.time.year()
        << setw(2) << 1 + l.time.month()
        << setw(2) << l.time.day()
        << ' '
        << setw(2) << l.time.hour() << ':'
        << setw(2) << l.time.min() << ':'
        << setw(2) << l.time.sec() << "."
        << setw(2) << l.time.usec()
        << ' '
        << setfill(' ') << setw(5)
        << ' '
        << l.filename << ':' << l.line_number << "]";
}

int main(int /*argc*/, char** argv) {
    // FLAGS_alsologtostderr = 1;
    google::SetLogDestination(google::GLOG_INFO, TEST_FOLDER "\\log_[");
    //google::SetLogDestination(google::GLOG_WARNING, TEST_FOLDER "\\log_[");
    google::SetLogFilenameExtension("].txt");
    // FLAGS_timestamp_in_logfile_name = false;

    // google::InitGoogleLogging(argv[0]);
    google::InitGoogleLogging(argv[0], &CustomPrefix);
    FLAGS_logbuflevel = -1; // this mean that INFO logs will cause flush every log

    int logLevel = FLAGS_logbuflevel;
    
    LOG(INFO) << "Test info 1";
    // H::Sleep(1000);
    LOG(INFO) << "Test info 2";
    LOG(INFO) << "Test info 3";
    LOG(WARNING) << "Test warning 1";
    LOG(WARNING) << "Test warning 2";
    LOG(INFO) << "Test info 4";
    LOG(INFO) << "Test info 5";

    //// H::Sleep(1000);
    //LOG(WARNING) << "Test warning 2";
    //LOG(WARNING) << "Test warning 3";
    //LOG(WARNING) << "Test warning 4";

    // RAW_LOG_INFO("Test");
    // RAW_LOG(INFO, "This is INFO");

    // Sleep(4000);
    return 0;
}



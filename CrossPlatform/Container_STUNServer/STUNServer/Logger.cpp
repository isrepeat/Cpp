#include "Logger.h"

Logger& Logger::GetInstance() {
    static Logger instance;
    return instance;
}

//void Logger::WriteInfo(std::string message) {
//    trace->P7_INFO(hModule, StringToWString(message).c_str());
//    //logger->Flush();
//}

//void Logger::WriteError(std::string message) {
//   /* trace->P7_ERROR(hModule, StringToWString(message).c_str());
//    logger->Flush();*/
//}
//
//void Logger::WriteDebug(std::string message) {
//    //trace->P7_DEBUG(hModule, StringToWString(message).c_str());
//    //logger->Flush();
//}
//
//void Logger::WriteTrace(std::string message) {
//    //trace->P7_TRACE(hModule, StringToWString(message).c_str());
//    //logger->Flush();
//}
//
Logger::~Logger() {
    logger->Flush();
    logger->Release();
    trace->Release();
}

Logger::Logger() {
#if defined(_WIN32) || defined(_WIN64)
    auto path = std::wstring(L"./");
    auto argument = L"/P7.Sink=FileTxt /P7.Format=\"%cn #%ix [%tf] %lv {%fs:%fl  %fn} %ms\" /P7.Dir=" + path + L" /P7.Files=1";
#else
    auto path = std::string("./");
    auto argument = "/P7.Sink=FileTxt /P7.Format=\"%cn #%ix [%tf] %lv {%fs:%fl  %fn} %ms\" /P7.Dir=" + path;
#endif
    logger = P7_Create_Client(argument.c_str());
    trace = P7_Create_Trace(logger, TM("Trace channel 1"));
    trace->Register_Thread(TM("VNCPrototype"), 0);
    trace->Register_Module(TM("Main"), &hModule);
    //P7_Set_Crash_Handler(); // Release logger manually in own crash handlers
}
#include "Platform.h"
#include "PlatformLogger.h"
#include "StunServer.h"

#if !defined(__CMAKE_BUILD__)
namespace {
	const std::filesystem::path appLocalFolder = ".";
}
#endif

int main() {
#if !defined(__CMAKE_BUILD__)
	lg::DefaultLoggers::Init(appLocalFolder / "StunServerMini.log", lg::InitFlags::DefaultFlags | lg::InitFlags::EnableLogToStdout);
#endif
	LOG_DEBUG_D("StunServer start");

	try {
		StunServer stunServer(53490);
		stunServer.StartListening();
	}
	catch (std::exception& ex) {
		LOG_ERROR_D("Catch exception = {}", ex.what());
	}

	LOG_DEBUG_D("StunServer exit");
	return 0;
}
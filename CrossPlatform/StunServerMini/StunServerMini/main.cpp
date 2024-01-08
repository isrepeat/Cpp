#include "Platform.h"
#include "StunServer.h"

namespace {
	const std::filesystem::path appLocalFolder = ".";
}

int main() {
	lg::DefaultLoggers::Init(appLocalFolder / "StunServerMini.log", lg::InitFlags::DefaultFlags | lg::InitFlags::EnableLogToStdout);
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
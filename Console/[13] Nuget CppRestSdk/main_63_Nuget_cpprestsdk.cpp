#include "Common.h"
#include "Tests.h"
#include <Helpers/Logger.h>

const std::filesystem::path g_TestOutputFolder =
    std::filesystem::path(__TEST_OUTPUT_FOLDER) / std::filesystem::path(__FILE__).stem();

int main() {
    _set_error_mode(_OUT_TO_MSGBOX);

    H::Flags<lg::InitFlags> loggerInitFlags =
        lg::InitFlags::DefaultFlags |
        lg::InitFlags::EnableLogToStdout;

    lg::DefaultLoggers::Init(g_TestOutputFolder / (g_TestOutputFolder.stem().string() + ".log"), loggerInitFlags);

    //TestStreams::TestWriteRead();
    //TestRequests::TestSimpleRequest();
    TestRequests::TestRequestsChain();
    system("pause");
	return 0;
}
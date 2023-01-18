#include "testCrashHandlingStatic.h"
#include <CrashHandling/CrashHandling.h>

void TestCrashHandlingAPI() {
	CrashHandling::OpenMinidumpChannel(NULL, L"");
}
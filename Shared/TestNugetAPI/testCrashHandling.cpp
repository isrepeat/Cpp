#include <iostream>
#include <CrashHandling/CrashHandling.h>
//#include "../TestNugetAPI_static/testCrashHandlingStatic.h"

int main() {
	
	EXCEPTION_POINTERS* pep = NULL;
	CrashHandling::OpenMinidumpChannel(pep, L"");
	//TestCrashHandlingAPI();

	return 0;
}
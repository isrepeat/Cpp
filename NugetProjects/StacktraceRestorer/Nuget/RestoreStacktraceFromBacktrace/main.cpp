#include <fstream>
#include <iostream>
#include "..\..\..\..\Shared\Helpers\Helpers.h"
#include "..\..\..\..\Shared\Helpers\Conversion.h"
#include "..\..\Nuget\StacktraceRestorer\StacktraceRestorer.h"


int wmain(int argc, wchar_t* argv[]) {
#if _DEBUG
	argc = 2;
	argv[0] = const_cast<wchar_t*>(L"...");
	argv[1] = const_cast<wchar_t*>(L"d:\\WORK\\TEST\\backtrace.txt");
#endif

	for (int i = 0; i < argc; i++) {
		wprintf(L"arg [%d] = %s \n", i, argv[i]);
	}
	wprintf(L"\n");

	if (argc < 2) {
		wprintf(L"Usage: RestoreStacktraceFromBacktrace.exe <path to offsets filename> \n");
		return 1;
	}

	std::wstring offsetsFilename = argv[1];
	auto backtrace = StacktraceRestorer::BackTrace(offsetsFilename);
	auto stacktraceFrames = StacktraceRestorer::BuildStacktrace(backtrace, H::FS::GetPathToFileW(offsetsFilename));


	wprintf(L"Stacktrace:\n\n");

	int counter = 0;
	std::wstring strStacktrace;
	for (auto& frame : stacktraceFrames) {
		wprintf(L"#%d %s:%d  %s \n\n", ++counter, frame.filename.c_str(), frame.lineNumber, frame.symbolName.c_str());
		//stacktrace += VecToWStr(buff);
	}

#if _DEBUG
	wprintf(L"\n\n");
	system("pause");
#endif
	return 0;
}
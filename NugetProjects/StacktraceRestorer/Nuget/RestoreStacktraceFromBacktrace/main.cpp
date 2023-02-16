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
	wprintf(L"\n\n");


	if (argc < 2) {
		wprintf(L"Usage: RestoreStacktraceFromBacktrace.exe <path to offsets filename>");
		return 0;
	}

	std::wstring offsetsFilename = argv[1];

	//// Read offsets from file to backtrace struct
	//std::wifstream inFile(pdbFolder + backtraceFilename); // backtrace file must be saved as UTF-8
	//std::vector<StacktraceRestorer::BacktraceFrame> backtraceFrames;

	////std::wstring pdbName = H::FS::GetFilenameFromPathW(argv[1]);

	//std::wstring moduleName, RVA;
	//while (inFile >> moduleName >> RVA) {
	//	//if (H::FS::RemoveExtFromFilenameW(moduleName) == H::RemoveExtFromFilename(pdbName)) { // keep offsets that associate only with pdbName
	//	//if (H::FS::RemoveExtFromFilenameW(moduleName) == L"TeamRemoteDesktop") {
	//	backtraceFrames.push_back({
	//			moduleName,
	//			H::HexFromStringW<std::uint32_t>(RVA)
	//			});
	//	//}
	//}

	//for (auto& frame : backtrace) {
	//	//wprintf(L"%s[0x%08x] + 0x%08x (VA = 0x%08x) \n", frame.moduleName.c_str(), frame.moduleBase, frame.RVA, frame.VA);
	//	wprintf(L"%s + 0x%08x \n", frame.moduleName.c_str(), frame.RVA);
	//}
	//wprintf(L"\n\n");


	auto backtrace = StacktraceRestorer::BackTrace(offsetsFilename);

	auto stacktrace = StacktraceRestorer::BuildStacktrace(backtrace, H::FS::GetPathToFileW(offsetsFilename));


	return 0;
}
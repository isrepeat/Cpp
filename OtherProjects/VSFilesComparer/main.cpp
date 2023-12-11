#include <Windows.h>
#include <iostream>
#include <filesystem>
#include <set>

bool ExecuteCommandLine(std::wstring parameters, bool admin = false, DWORD showFlag = SW_SHOW) {
	SHELLEXECUTEINFO ShExecInfo = { 0 };
	ShExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
	ShExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
	ShExecInfo.hwnd = NULL;
	ShExecInfo.lpVerb = admin ? L"runas" : L"open";
	ShExecInfo.lpFile = L"cmd.exe";
	ShExecInfo.lpParameters = parameters.c_str();
	ShExecInfo.lpDirectory = NULL;
	ShExecInfo.nShow = showFlag;
	ShExecInfo.hInstApp = NULL;
	bool res = ShellExecuteExW(&ShExecInfo);
	WaitForSingleObject(ShExecInfo.hProcess, INFINITE);
	return res;
}

int wmain(int argc, wchar_t* argv[]) {
	if (argc < 3) {
		printf(
			"Usage VSFilesComparer <path_to_folder_1> <path_to_folder_2> \n"
			"NOTE: Require VS 2019 \n"
		);
		return 0;
	}
	
	//setlocale(LC_CTYPE, "");
	//std::wstring pathToDevenv = L"\"%ProgramFiles(x86)%\\Microsoft Visual Studio\\2019\\Community\\Common7\\IDE\\devenv.exe\"";
	std::wstring pathToDevenv = L"\"c:\\Program Files (x86)\\Microsoft Visual Studio\\2019\\Community\\Common7\\IDE\\devenv.exe\"";

	std::wstring pathOne = argv[1];
	std::wstring pathTwo = argv[2];
	//std::wstring pathOne = L"D:\\WORK\\STUNServer\\STUNServer_27_06_2022\\STUNServer\\";
	//std::wstring pathTwo = L"D:\\WORK\\STUNServer\\STUNServer_Test_last\\";

	wprintf(L"args ... pathOne = %s, pathTwo = %s \n", pathOne.c_str(), pathTwo.c_str());


	std::set<std::wstring> setOne;
	std::set<std::wstring> setTwo;

	for (const auto& entry : std::filesystem::directory_iterator(pathOne)) {
		setOne.insert(entry.path().filename());
	}

	std::cout << "\n\n";

	for (const auto& entry : std::filesystem::directory_iterator(pathTwo)) {
		//std::cout << entry.path() << "..." << entry.path().filename() << std::endl;
		setTwo.insert(entry.path().filename());
	}


	for (auto& itemOne : setOne) {
		for (auto& itemTwo : setTwo) {
			if (itemTwo == itemOne) {
				auto command = std::wstring{ L"/c " + pathToDevenv + L" /diff " } + pathOne + itemOne + L" " +pathTwo + itemTwo;
				ExecuteCommandLine(command);
				break;
			}
		}
	}

	std::cout << "\n\n";
	//system("pause");
	return 0;
}
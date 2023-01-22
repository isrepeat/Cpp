#include <Windows.h>
#include "../Shared/Helpers/Helpers.h"
#include <map>

int main() {
	std::wstring params = H::CreateStringParams({
				{L"-processId", std::to_wstring(GetCurrentProcessId())},
				{L"-minidumpPath", {}},
				{L"-sleepAtStart", L"15000"},
				//{L"-isUWP", L"false"},
		});

	params += L"-isUWP";

	auto listArgs = H::ParseArgsFromString(params);
	for (auto& arg : listArgs) {
		wprintf(L"parsed .. %s \n", arg.c_str());
	}

	wprintf(L"\n");
	auto pairArgs = H::ParseArgsFromStringToPair(params);
	for (auto& arg : pairArgs) {
		wprintf(L"pair .. name = %s, value = %s \n", arg.first.c_str(), arg.second.c_str());
	}

	wprintf(L"\n");
	auto mappedArgs = H::ParseArgsFromStringToMap(params);
	if (!mappedArgs.count(L"-isUWP")) {
		wprintf(L"error: missed 'isUWP' flag \n");
	}
	for (auto& item : mappedArgs) {
		wprintf(L"map .. name = %s, value = %s \n", item.first.c_str(), item.second.c_str());
	}

	//system("pause");
	Sleep(7000);
	return 0;
}

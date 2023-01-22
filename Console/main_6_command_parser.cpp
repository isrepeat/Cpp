#include <Windows.h>
#include "../Shared/Helpers/Helpers.h"


int main() {
	std::wstring params = H::CreateStringParams({
				{L"-processId", std::to_wstring(GetCurrentProcessId())},
				{L"-minidumpPath", {}},
				{L"-sleepAtStart", L"15000"}
		});

	auto listArgs = H::ParseArgsFromString(params);
	for (auto& arg : listArgs) {
		wprintf(L"parsed .. %s \n", arg.c_str());
	}

	wprintf(L"\n");
	auto pairArgs = H::ParseArgsFromStringToPair(params);
	for (auto& arg : pairArgs) {
		wprintf(L"pair .. name = %s, value = %s \n", arg.first.c_str(), arg.second.c_str());
	}

	system("pause");
	return 0;
}

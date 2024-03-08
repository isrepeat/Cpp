#include <Helpers/SystemInfo.h>
#include <Windows.h>
#include <iostream>

int main() {
	auto computerName = H::GetComputerNameW();
	std::wcout << computerName;
	Sleep(10'000);
	return 0;
}
#pragma once
#include <Windows.h>
#include <string>

namespace H {
	class HardDrive {
	public:
		static long double GetFreeMemoryInGB(const std::wstring& disk);
		static unsigned long long GetFreeMemory(const std::wstring& disk); // [Bytes]
		static unsigned long long GetFilesize(const std::wstring& disk); // [Bytes]

		static std::wstring GetDiskLetterFromPath(const std::wstring& path);
	};
}
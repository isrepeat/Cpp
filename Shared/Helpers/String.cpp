#pragma once
#include "String.h"
#include <Windows.h>

namespace H {
	void DebugOutput(const std::wstring& msg) {
		OutputDebugStringW((msg + L"\n").c_str());
	}

	void DebugOutput(const std::string& msg) {
		OutputDebugStringA((msg + "\n").c_str());
	}
}
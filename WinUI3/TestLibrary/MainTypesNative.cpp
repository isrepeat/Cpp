#include "MainTypesNative.h"

namespace TestLibraryNative {
	int MainTypesNative::GetTestInt() {
		return 1234;
	}
	void MainTypesNative::SetTestInt(int value) {
		return;
	}

	std::string MainTypesNative::GetTestString() {
		return "string";
	}
	std::wstring MainTypesNative::GetTestWString() {
		return L"wstring";
	}
}
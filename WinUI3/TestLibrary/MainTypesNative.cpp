#include "MainTypesNative.h"

namespace TestLibraryNative {
	int MainTypesNative::GetTestInt() {
		return 3333;
	}
	void MainTypesNative::SetTestInt(int value) {
		this->testInt = value;
	}

	std::wstring MainTypesNative::GetTestWString() {
		return this->testWString;
	}
	void MainTypesNative::SetTestWString(std::wstring str) {
		this->testWString = str;
	}
}
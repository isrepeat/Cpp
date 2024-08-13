#pragma once
#include <string>
#include <vector>
#include <memory>

namespace TestLibraryNative {
	class MainTypesNative {
	public:
		MainTypesNative() = default;
		~MainTypesNative() = default;

		int GetTestInt();
		void SetTestInt(int value);

		std::wstring GetTestWString();
		void SetTestWString(std::wstring str);

	private:
		int testInt = 0;
		std::wstring testWString;
		std::vector<int> testVector;
	};
}
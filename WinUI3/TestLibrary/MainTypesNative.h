#pragma once
#include <string>
#include <memory>

namespace TestLibraryNative {
	class MainTypesNative {
	public:
		MainTypesNative() = default;
		~MainTypesNative() = default;

		int GetTestInt();
		void SetTestInt(int value);

		std::string GetTestString();
		std::wstring GetTestWString();
	};
}
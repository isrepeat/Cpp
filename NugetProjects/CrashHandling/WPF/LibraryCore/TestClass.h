#pragma once

namespace LibraryCore {
	class TestClass {
	//public:
	//	TestClass();

	private:
		TestClass();

	public:
		static TestClass& GetInstance();
	};
}
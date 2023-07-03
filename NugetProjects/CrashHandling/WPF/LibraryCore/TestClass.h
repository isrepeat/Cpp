#pragma once

namespace LibraryCore {
	class TestClass {
	public:
		void Invoke();

	private:
		TestClass();

	public:
		static TestClass& GetInstance();
	};
}
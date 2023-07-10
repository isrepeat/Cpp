#pragma once
#include "../LibraryCore/TestClass.h"

namespace LibraryCLR {
	public ref class TestClassCLR {
	public:
		void InvokeCLR();

	private:
		TestClassCLR();


	public:
		static TestClassCLR^ GetInstance();


	private:
		static TestClassCLR^ m_instance;
		//LibraryCore::TestClass* testClassCore;
	};
}
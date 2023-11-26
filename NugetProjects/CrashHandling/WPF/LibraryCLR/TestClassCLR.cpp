#include "TestClassCLR.h"
#include <Windows.h>
#include "../../../../Shared/Helpers/Helpers.h"
#include "../../../../Shared/Helpers/Conversion.h"
//#include <CrashHandling/CrashHandling.h>


namespace LibraryCLR {

	TestClassCLR^ TestClassCLR::GetInstance() {
		if (m_instance == nullptr)
			m_instance = gcnew TestClassCLR();

		return m_instance;
	}

	void TestClassCLR::InvokeCLR() {
		int aaa = 123;
		LibraryCore::TestClass::GetInstance().Invoke();
		int bbb = aaa + 345;
	}

	TestClassCLR::TestClassCLR() {
		LibraryCore::TestClass::GetInstance();
	}
}
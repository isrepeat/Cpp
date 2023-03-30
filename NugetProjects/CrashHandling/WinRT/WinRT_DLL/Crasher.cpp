#include "Crasher.h"
#include <Windows.h>

using namespace WinRT_DLL;
using namespace Platform;

void Bar() {
	//int aaa = 9;

	int* ptr = new int{ 1 };
	delete ptr;

	*ptr = 17;


	//__try {
	//	*(int*)0 = 0;
	//}
	//__except (MyCustomExceptionHandler(GetExceptionInformation())) {
	//}

	int bbb = 9;
}

Crasher::Crasher()
	: packageFolder{ Windows::Storage::ApplicationData::Current->LocalFolder->Path->Data() }
	, installedFolder{ Windows::ApplicationModel::Package::Current->InstalledLocation->Path->Data() }
{		
	int xx = 9;
}

void WinRT_DLL::Crasher::AccessViolation() {
	Bar();
	int xx = 9;
}

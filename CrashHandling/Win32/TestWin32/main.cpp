#include <CrashHandling/CrashHandling.h>
#include "../../../Shared/ComAPI/ComAPI.h";
#include "../../../Shared/Helpers/Helpers.h";
#include <Windows.h>
#include <thread>


LONG __stdcall MyCustomExceptionHandler(EXCEPTION_POINTERS* pep) {
	printf("MyCustomFilter start \n");

	auto packageFolder = ComApi::GetPackageFolder();
	auto installFolder = H::ExePathW();

	std::wstring protcolWithParams = L"/c start testminidumpwriter:\"" + H::CreateStringParams({
				{L"-processId", std::to_wstring(GetCurrentProcessId())},
				{L"-minidumpPath", packageFolder},
				{L"-sleepAtStart", L"100"}
		}) + L"\"";

	auto res = H::ExecuteCommandLine(protcolWithParams, false, SW_HIDE);



	//while (true) {
	//	Sleep(100);
	//}
	CrashHandling::OpenMinidumpChannel(pep, packageFolder);

	printf("MyCustomFilter end \n");

	Sleep(8000);
	exit(1);
	return EXCEPTION_EXECUTE_HANDLER; //EXCEPTION_CONTINUE_EXECUTION
}


void Foo() {
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

int main() {
	CrashHandling::RegisterVectorHandler(&MyCustomExceptionHandler);

	Foo();

	while (true) {
		Sleep(100);
	}
	return 0;
}
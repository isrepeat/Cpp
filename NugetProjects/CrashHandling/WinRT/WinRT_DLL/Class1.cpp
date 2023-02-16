#include "Class1.h"
#include <Windows.h>
#include <ppltasks.h>
#include <shellapi.h>
#include <errhandlingapi.h>
#include <filesystem>
#include <fstream>
#include <string>
#include "../../../Shared/HelpersWinRT/Helpers.h"
#include <CrashHandling/CrashHandling.h>

using namespace WinRT_DLL;
using namespace Platform;

LONG __stdcall MyCustomExceptionHandler(EXCEPTION_POINTERS* pep) {
	std::wstring protcolWithParams = L"test-minidumpwriter:\"" + HelpersWinRT::CreateStringParams({
				{L"-isUWP", {}},
				{L"-processId", std::to_wstring(GetCurrentProcessId())},
				{L"-sleepAtStart", L"100"},
		}) + L"\"";

	auto uri = ref new Windows::Foundation::Uri(ref new Platform::String(protcolWithParams.c_str()));
	Windows::System::Launcher::LaunchUriAsync(uri);

	std::wstring packageFolder = Windows::Storage::ApplicationData::Current->LocalFolder->Path->Data();
	CrashHandling::OpenMinidumpChannel(pep, packageFolder);

	Sleep(8000);
	exit(1);
	return EXCEPTION_EXECUTE_HANDLER; //EXCEPTION_CONTINUE_EXECUTION
}



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

Class1::Class1() 
	: packageFolder{ Windows::Storage::ApplicationData::Current->LocalFolder->Path->Data() }
	, installedFolder{ Windows::ApplicationModel::Package::Current->InstalledLocation->Path->Data() }
{	
	// TODO: test LaunchFullTrustProcessForCurrentAppWithArgumentsAsync() with Windows SDK 22000
	//if (Windows::Foundation::Metadata::ApiInformation::IsApiContractPresent("Windows.ApplicationModel.FullTrustAppContract", 1, 0)) {
	//	Windows::ApplicationModel::FullTrustProcessLauncher::LaunchFullTrustProcessForCurrentAppAsync();
	//}





	CrashHandling::RegisterVectorHandler(&MyCustomExceptionHandler);
	
	Bar();
	
	Sleep(2000);
	Sleep(2000);
	int xx = 9;
}

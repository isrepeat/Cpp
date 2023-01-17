#include "Class1.h"
#include <Windows.h>
#include <shellapi.h>
#include <errhandlingapi.h>
#include <filesystem>
#include <fstream>
#include <string>
#include <CrashHandling/CrashHandling.h>
//#include "../../CrashHandlingAPI_DLL/CrashHandlingAPI.h"
//#pragma comment (lib, "../../../!VS_TMP/Build/v142/Debug/x64/CrashHandlingAPI_DLL.lib")



#include "../../../Shared/HelpersWinRT/Helpers.h"

using namespace WinRT_DLL;
using namespace Platform;
//using namespace HelpersWinRT;


LONG __stdcall MyCustomExceptionHandler(EXCEPTION_POINTERS* pep)
{
	printf("MyCustomFilter start \n");
	//std::wstring exePath = Windows::ApplicationModel::Package::Current->InstalledLocation->Path->Data();
	//std::wstring command = L"/c " + exePath + L"\\MiniDumpWriter.exe";
	//CrashAPI::ExecuteCommandLine(command.c_str(), false, SW_SHOW);


	std::wstring packageFolder = Windows::Storage::ApplicationData::Current->LocalFolder->Path->Data();
	std::wstring protcolWithParams = L"dumpwriter:\"" + HelpersWinRT::CreateStringParams({
				{L"-processId", std::to_wstring(GetCurrentProcessId())},
				{L"-minidumpPath", packageFolder}
		}) + L"\"";

	//auto exePath = HelpersWinRT::ExePathW();
	auto uri = ref new Windows::Foundation::Uri(ref new Platform::String(protcolWithParams.c_str()));
	Windows::System::Launcher::LaunchUriAsync(uri);

	CrashHandling::OpenMinidumpChannel(pep, packageFolder);

	printf("MyCustomFilter end \n");

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
	CrashHandling::RegisterVectorHandler(&MyCustomExceptionHandler);

	////auto win32ProcessId = GetCurrentProcessId();
	////auto processId = Windows::System::Diagnostics::ProcessDiagnosticInfo::GetForCurrentProcess()->ProcessId;
	//std::wstring protcolWithParams = L"dumpwriter:\"" + HelpersWinRT::CreateStringParams({
	//			{L"-threadId", std::to_wstring(GetCurrentProcessId())}
	//	}) + L"\"";

	//auto exePath = HelpersWinRT::ExePathW();
	//auto uri = ref new Windows::Foundation::Uri(ref new Platform::String(protcolWithParams.c_str()));
	//Windows::System::Launcher::LaunchUriAsync(uri);

	//

	////auto win32CurrentThreadId = GetCurrentThreadId();
	////CrashAPI::CreateMinidumpChannel(1488, nullptr);
	//CrashAPI::OpenMinidumpChannel();


	Bar();


	int aa = 9;
	Sleep(2000);
	Sleep(2000);
	int xx = 9;
	//Bar();
}

#pragma once
#include <string>
#include <comdef.h>
#include <Wbemidl.h>
#include <sysinfoapi.h>
#include <cmath>
#include <wrl.h>

#ifdef SYSTEMINFOFOROFFICELIB_EXPORTS
#define SYSTEM_INFO_API __declspec(dllexport)
#else
#define SYSTEM_INFO_API __declspec(dllimport)
#endif

#pragma comment(lib, "wbemuuid.lib")

enum DataType
{
	Architecture,
	OS,
	Region,
	Language,
	RAM
};

namespace SystemDataProvider
{
	namespace {
	
		static std::string os;
		static std::string arch;
		static std::string region;
		static std::string language;
		static int ram;
	
		std::string ws2s(const std::wstring& s)
		{
			if (s.length() == 0)
				return std::string{};
			int len;
			int slength = (int)s.length() + 1;
			len = WideCharToMultiByte(CP_ACP, 0, s.c_str(), slength, 0, 0, 0, 0);
			std::string r(len, '\0');
			WideCharToMultiByte(CP_ACP, 0, s.c_str(), slength, &r[0], len, 0, 0);
			return r;
		}
	
	}

	void FetchOsInfo()
	{
		SYSTEM_INFO sysInfo;
		GetNativeSystemInfo(&sysInfo);
		if (sysInfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_INTEL)
			arch = "x86";
		else if (sysInfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64)
			arch = "x64";
		else
			arch = "unknown";

		OSVERSIONINFOA version;
		version.dwOSVersionInfoSize = sizeof(OSVERSIONINFOA);

		GetVersionExA(&version);
		const uint16_t majorVersion = version.dwMajorVersion == 10 && version.dwBuildNumber > 22'000 ? 11 : version.dwMajorVersion;

		os = "Windows " + std::to_string(majorVersion) + " "
			+ std::to_string(version.dwBuildNumber);
	}

	void FetchLangInfo() {
		int ret;

		char tmp[100];
		ret = GetLocaleInfoA(LOCALE_USER_DEFAULT, LOCALE_SENGLISHCOUNTRYNAME, (LPSTR)tmp,
			sizeof(tmp) / sizeof(char));

		region = { tmp };
		char tmpLang[20];

		ret = GetLocaleInfoA(LOCALE_USER_DEFAULT, LOCALE_SISO639LANGNAME, (LPSTR)tmpLang,
			sizeof(tmpLang) / sizeof(char));
		language = { tmpLang };
	}

	bool InitObjects()
	{
		HRESULT hres;

		hres = CoInitializeEx(0, COINIT_APARTMENTTHREADED);
		if (FAILED(hres))
		{
			return false;
		}

		FetchOsInfo();

		Microsoft::WRL::ComPtr<IWbemLocator> pLoc = NULL;

		hres = CoCreateInstance(CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER, IID_IWbemLocator,
			(LPVOID*)&pLoc);

		if (FAILED(hres))
		{
			CoUninitialize();
			return false;
		}

		Microsoft::WRL::ComPtr<IWbemServices> pSvc = NULL;

		hres = pLoc->ConnectServer(_bstr_t(L"ROOT\\CIMV2"), NULL, NULL, 0, NULL, 0, 0, &pSvc);

		if (FAILED(hres))
		{
			CoUninitialize();
			return false;
		}

		hres = CoSetProxyBlanket(pSvc.Get(), RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, NULL,
			RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE);

		if (FAILED(hres))
		{
			CoUninitialize();
			return false;
		}

		Microsoft::WRL::ComPtr<IEnumWbemClassObject> pEnumerator = NULL;
		hres = pSvc->ExecQuery(bstr_t("WQL"), bstr_t("SELECT * FROM Win32_OperatingSystem"),
			WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, NULL,
			&pEnumerator);

		if (FAILED(hres))
		{
			CoUninitialize();
			return false;
		}

		Microsoft::WRL::ComPtr<IWbemClassObject> pclsObj = NULL;
		ULONG uReturn = 0;

		while (pEnumerator)
		{
			HRESULT hr = pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn);

			if (0 == uReturn)
			{
				break;
			}

			VARIANT vtProp;

			//hr = pclsObj->Get(L"Name", 0, &vtProp, 0, 0);
			//os = vtProp.bstrVal;
			//VariantClear(&vtProp);
			//os = os.substr(0, os.find(L"|")) + L" ";

			///*hr = pclsObj->Get(L"OSArchitecture", 0, &vtProp, 0, 0);
			//arch = vtProp.bstrVal;
			//VariantClear(&vtProp);*/

			/* hr = pclsObj->Get(L"Version", 0, &vtProp, 0, 0);
			os += ws2s(vtProp.bstrVal);
			VariantClear(&vtProp);*/

			hr = pclsObj->Get(L"TotalVisibleMemorySize", 0, &vtProp, 0, 0);
			auto bytesString = vtProp.bstrVal;
			ram = (int)std::floor((float)_wtoi(bytesString) / 1000000);
			VariantClear(&vtProp);
		}

		FetchLangInfo();

		return true;
	}


	void Deinit() { CoUninitialize(); }

	std::string Get(DataType val)
	{
		switch (val)
		{
		case Architecture:
			return arch;
		case OS:
			return os;
		case DataType::Region:
			return region;
		case Language:
			return language;
		case RAM:
			return std::to_string(ram) + " GB";
		default:
			return std::string();
		}
	}
} // namespace SystemDataProvider

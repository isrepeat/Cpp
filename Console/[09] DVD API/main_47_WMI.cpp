#define _WIN32_DCOM
#include <iostream>
#include <comdef.h>
#include <Wbemidl.h>

#pragma comment(lib, "wbemuuid.lib")


//namespace Utility
//{
	enum class eFileSystem {
		FS_UDF,
		FS_FAT,
		FS_FAT32,
		FS_NTFS,
	};

	const WCHAR* GetFileSystemStr(eFileSystem fs)
	{
		switch (fs)
		{
		case eFileSystem::FS_UDF:
			return L"UDF";
		case eFileSystem::FS_FAT:
			return L"FAT";
		case eFileSystem::FS_FAT32:
			return L"FAT32";
		case eFileSystem::FS_NTFS:
		default:
			return L"NTFS";
		}
	}

	HRESULT WMI_FormatVolume(BYTE volumeLetter, const TCHAR* szLabel, int* pErrorCode, eFileSystem fs, bool bQuickFormat, LONG clusterSize, bool bEnableCompression)
	{
		HRESULT hr = S_OK;

		hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
		if (S_OK != hr)
			return hr;

		IWbemLocator* pLocator = NULL;
		IWbemServices* pService = NULL;
		IWbemClassObject* pClass = NULL;
		IWbemClassObject* pInParamsDefinition = NULL;
		IWbemClassObject* pClassInstance = NULL;
		IEnumWbemClassObject* pEnumerator = NULL;

		do
		{
			hr = CoInitializeSecurity(NULL, -1, NULL, NULL, RPC_C_AUTHN_LEVEL_DEFAULT, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE, NULL);
			if (S_OK != hr)
				break;

			hr = CoCreateInstance(CLSID_WbemLocator, NULL, CLSCTX_INPROC_SERVER, IID_IWbemLocator, reinterpret_cast<LPVOID*>(&pLocator));
			if (S_OK != hr)
				break;

			hr = pLocator->ConnectServer(_bstr_t(L"ROOT\\CIMV2"), NULL, NULL, NULL, 0, NULL, NULL, &pService);
			if (S_OK != hr)
				break;

			hr = CoSetProxyBlanket(pService, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, NULL, RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE);
			if (S_OK != hr)
				break;

			_bstr_t bstrMethod(L"Format");
			_bstr_t bstrClassName(L"Win32_Volume");

			// get class and method here
			hr = pService->GetObject(bstrClassName, 0, NULL, &pClass, NULL);
			if (S_OK != hr)
				break;

			hr = pClass->GetMethod(bstrMethod, 0, &pInParamsDefinition, NULL);
			if (S_OK != hr)
				break;

			hr = pInParamsDefinition->SpawnInstance(0, &pClassInstance);
			if (S_OK != hr)
				break;

			// execute QWL to query volume info
			WCHAR szWQL[128] = L""; swprintf_s(szWQL, L"SELECT * FROM Win32_Volume WHERE DriveLetter = '%c:'", volumeLetter);
			hr = pService->ExecQuery(_bstr_t(L"WQL"), _bstr_t(szWQL), WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, NULL, &pEnumerator);
			if (S_OK != hr)
				break;

			while (pEnumerator)
			{
				ULONG uReturn = 0;
				IWbemClassObject* pVolumeObj = NULL;
				hr = pEnumerator->Next(WBEM_INFINITE, 1, &pVolumeObj, &uReturn);
				if (0 == uReturn)
					break;

				// set format param
				_variant_t varFileSystem(GetFileSystemStr(fs));
				_variant_t varQuickFormat(bQuickFormat);
				_variant_t varClusterSize(clusterSize);
				_variant_t varLabel(szLabel);
				_variant_t varEnableCompression(bEnableCompression);
				hr = pClassInstance->Put(L"FileSystem", 0, &varFileSystem, 0);
				hr = pClassInstance->Put(L"QuickFormat", 0, &varQuickFormat, 0);
				hr = pClassInstance->Put(L"ClusterSize", 0, &varClusterSize, 0);
				hr = pClassInstance->Put(L"Label", 0, &varLabel, 0);
				hr = pClassInstance->Put(L"EnableCompression", 0, &varEnableCompression, 0);

				// key!!! get class object 
				_variant_t var;
				hr = pVolumeObj->Get(L"__PATH", 0, &var, NULL, NULL);
				pVolumeObj->Release();

				if (S_OK != hr)
					break;

				IWbemClassObject* pOutParams = NULL;
				hr = pService->ExecMethod(var.bstrVal, bstrMethod, 0, NULL, pClassInstance, &pOutParams, NULL);
				if (S_OK != hr)
					break;

				_variant_t varReturnValue;
				pOutParams->Get(L"ReturnValue", 0, &varReturnValue, NULL, 0);
				pOutParams->Release();

				int formatResult = varReturnValue.intVal;
				if (0 != formatResult)
					hr = S_FALSE;

				if (NULL != pErrorCode)
					*pErrorCode = formatResult;
			}

		} while (0);

		if (NULL != pLocator)
			pLocator->Release();

		if (NULL != pService)
			pService->Release();

		if (NULL != pClass)
			pClass->Release();

		if (NULL != pInParamsDefinition)
			pInParamsDefinition->Release();

		if (NULL != pClassInstance)
			pClassInstance->Release();

		if (NULL != pEnumerator)
			pEnumerator->Release();

		CoUninitialize();

		return hr;
	}
//}

int main() {
	int errorCode = 0;
	WMI_FormatVolume('f', L"Label_UDF", &errorCode, eFileSystem::FS_UDF, true, 4096, false);
	return 0;
}
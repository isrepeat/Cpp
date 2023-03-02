#define BUILD_LIBRARY
#define STATIC_LIB

#include "ComAPI.h"

#include <Windows.Services.Store.h>
#include <condition_variable>
#include <windows.storage.h>
#include <windows.system.profile.h>
#include <Windows.ApplicationModel.h>
#include <Windows.ApplicationModel.datatransfer.h>
#include <shobjidl_core.h>
#include <Winerror.h>
#include <Shlobj.h>
#include <roapi.h>
#include <vector>
#include <format>
#include <wrl.h>

// TODO: initialize COM once like singleton ...
// TODO: Rename this library to some name associated with COM

using namespace Microsoft::WRL;
using namespace Microsoft::WRL::Wrappers;
using namespace ABI::Windows::System::Profile;
using namespace ABI::Windows::Services::Store;
using namespace ABI::Windows::Foundation;
using namespace ABI::Windows::Foundation::Collections;
using namespace ABI::Windows::Storage;
using namespace ABI::Windows::ApplicationModel;
using namespace ABI::Windows::ApplicationModel::DataTransfer;

class CCoInitialize {
	HRESULT m_hr;

public:
	CCoInitialize() : m_hr(CoInitialize(NULL)) 
	{}
	~CCoInitialize() {
		if (SUCCEEDED(m_hr)) CoUninitialize(); 
	}
	operator HRESULT() const { 
		return m_hr;
	}
};

#define CheckHr(hr) do { if (FAILED(hr)) __debugbreak(); } while (false)

namespace ComApi {
	// Absolute path to the App package
	std::wstring API GetPackageFolder() {
		CCoInitialize tmpComInit;
		ComPtr<IApplicationDataStatics> appDataStatic;
		auto hr = RoGetActivationFactory(HStringReference(RuntimeClass_Windows_Storage_ApplicationData).Get(), __uuidof(appDataStatic), &appDataStatic);
		CheckHr(hr);

		ComPtr<IApplicationData> appData;
		hr = appDataStatic->get_Current(appData.GetAddressOf());
		CheckHr(hr);

		ComPtr<IStorageFolder> localFolder;
		hr = appData->get_LocalFolder(localFolder.GetAddressOf());
		CheckHr(hr);


		ComPtr<IStorageItem> item;
		hr = localFolder.As(&item);
		CheckHr(hr);

		//HSTRING myPath = nullptr;
		//item->get_Path(&myPath);

		//HString hstr;
		//hstr.Set(myPath);

		//return hstr.GetRawBuffer(NULL);

		HString pacakgeFolderHstr;
		item->get_Path(pacakgeFolderHstr.GetAddressOf());
		std::wstring packageFolder = pacakgeFolderHstr.GetRawBuffer(NULL);
		return packageFolder;
	}

	std::wstring API WindowsVersion() {
		CCoInitialize tmpComInit;
		ComPtr<ABI::Windows::System::Profile::IAnalyticsInfoStatics> analyticsInfoStatic;
		auto hr = RoGetActivationFactory(HStringReference(RuntimeClass_Windows_System_Profile_AnalyticsInfo).Get(), __uuidof(analyticsInfoStatic), &analyticsInfoStatic);
		CheckHr(hr);

		ComPtr<IAnalyticsVersionInfo> analyticsInfo;
		hr = analyticsInfoStatic->get_VersionInfo(analyticsInfo.GetAddressOf());
		CheckHr(hr);

		HString deviceFamilyVersionHstr;
		analyticsInfo->get_DeviceFamilyVersion(deviceFamilyVersionHstr.GetAddressOf());
		std::wstring deviceFamilyVersion = deviceFamilyVersionHstr.GetRawBuffer(NULL);

		uint64_t version = std::stoll(deviceFamilyVersion);
		uint16_t major = (version & 0xFFFF000000000000L) >> 48;
		uint16_t minor = (version & 0x0000FFFF00000000L) >> 32;
		uint16_t build = (version & 0x00000000FFFF0000L) >> 16;
		uint16_t revision = (version & 0x000000000000FFFFL);

		return std::format(L"{}.{}.{}.{}", major, minor, build, revision);
	}
}
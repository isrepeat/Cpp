#include "pch.h"
#include "PackageProvider.h"
#include <libhelpers/HText.h>
#include <libhelpers/HSystem.h>
#include <Windows.h>
#include <windows.storage.h>
#include <appmodel.h> // must be included after Windows.h
#include <objbase.h>
#include <wrl.h>
#include <vector>
#include <condition_variable>

#pragma comment(lib, "RuntimeObject.lib")

using namespace Microsoft::WRL;
using namespace Microsoft::WRL::Wrappers;
using namespace ABI::Windows::Storage;

#ifndef Dbreak
#ifdef _DEBUG
#define Dbreak              \
if (IsDebuggerPresent()) {  \
    __debugbreak();         \
}
#else
#define Dbreak
#endif
#endif

#define CheckHr(hr) do { if (FAILED(hr)) Dbreak; } while (false)

#if HAVE_WINRT == 0
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
#endif

namespace H {
    bool PackageProvider::IsRunningUnderPackage() {
        UINT32 length = 0;
        LONG rc = GetCurrentPackageFamilyName(&length, NULL);
        if (rc == APPMODEL_ERROR_NO_PACKAGE) {
            return false;
        }
        return true;
    }

    // Absolute path to the App package
    std::wstring PackageProvider::GetPackageFolder() {
#if HAVE_WINRT == 0
        CCoInitialize tmpComInit;
#endif
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

        HString pacakgeFolderHstr;
        item->get_Path(pacakgeFolderHstr.GetAddressOf());
        std::wstring packageFolder = pacakgeFolderHstr.GetRawBuffer(NULL);
        return packageFolder;
    }

    std::wstring PackageProvider::GetApplicationUserModelId() {
        UINT32 length = 0;
        LONG rc = GetCurrentApplicationUserModelId(&length, NULL);
        if (rc != ERROR_INSUFFICIENT_BUFFER)
        {
            if (rc == APPMODEL_ERROR_NO_PACKAGE)
                wprintf(L"Process has no package identity\n");
            else
                wprintf(L"Error %d in GetCurrentPackageFamilyName\n", rc);

            return L"";
        }

        std::vector<wchar_t> familyName(length);

        rc = GetCurrentApplicationUserModelId(&length, familyName.data());
        if (rc != ERROR_SUCCESS)
        {
            wprintf(L"Error %d retrieving PackageFamilyName\n", rc);
            return L"";
        }

        return H::Text::VecToWStr(familyName);
    }
}
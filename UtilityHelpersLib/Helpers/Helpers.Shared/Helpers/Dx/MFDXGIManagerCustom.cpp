#include "MFDXGIManagerCustom.h"

namespace HELPERS_NS {
    namespace Dx {
        MFDXGIDeviceManagerCustom::MFDXGIDeviceManagerCustom(Microsoft::WRL::ComPtr<IMFDXGIDeviceManager> mfDxgiDeviceManagerOrig)
            : mfDxgiDeviceManagerOrig{ mfDxgiDeviceManagerOrig }
        {}

        HRESULT __stdcall MFDXGIDeviceManagerCustom::CloseDeviceHandle(HANDLE hDevice) {
            return mfDxgiDeviceManagerOrig->CloseDeviceHandle(hDevice);
        }

        HRESULT __stdcall MFDXGIDeviceManagerCustom::GetVideoService(HANDLE hDevice, REFIID riid, void** ppService) {
            return mfDxgiDeviceManagerOrig->GetVideoService(hDevice, riid, ppService);
        }

        HRESULT __stdcall MFDXGIDeviceManagerCustom::LockDevice(HANDLE hDevice, REFIID riid, void** ppUnkDevice, BOOL fBlock) {
            HRESULT hr = S_OK;
            hr = mfDxgiDeviceManagerOrig->LockDevice(hDevice, riid, ppUnkDevice, fBlock);
            return hr;
        }

        HRESULT __stdcall MFDXGIDeviceManagerCustom::OpenDeviceHandle(HANDLE* phDevice) {
            return mfDxgiDeviceManagerOrig->OpenDeviceHandle(phDevice);
        }

        HRESULT __stdcall MFDXGIDeviceManagerCustom::ResetDevice(IUnknown* pUnkDevice, UINT resetToken) {
            return mfDxgiDeviceManagerOrig->ResetDevice(pUnkDevice, resetToken);
        }

        HRESULT __stdcall MFDXGIDeviceManagerCustom::TestDevice(HANDLE hDevice) {
            return mfDxgiDeviceManagerOrig->TestDevice(hDevice);
        }

        HRESULT __stdcall MFDXGIDeviceManagerCustom::UnlockDevice(HANDLE hDevice, BOOL fSaveState) {
            HRESULT hr = mfDxgiDeviceManagerOrig->UnlockDevice(hDevice, fSaveState);
            return hr;
        }
    }
}
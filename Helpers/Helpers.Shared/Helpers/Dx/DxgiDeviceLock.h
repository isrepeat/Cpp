#pragma once
#include <Helpers/common.h>
#include <Helpers/UniqueHandle.h>
#include <Helpers/System.h>
#include <Helpers/Memory.h>
#include <Helpers/Macros.h>
#include <mfobjects.h>
#include <d3d11_1.h>
#include <d3d11.h>
#include <wrl.h>

namespace HELPERS_NS {
	namespace Dx {
		template<class DeviceManagerT>
		class DxgiDeviceLockBase {
		protected:
			DxgiDeviceLockBase(Microsoft::WRL::ComPtr<DeviceManagerT> deviceManager)
				: deviceManager{ deviceManager }
				, hDevice{ nullptr }
				, locked{ false }
			{
				if (this->deviceManager) {
					this->deviceManager->OpenDeviceHandle(&hDevice);
				}
			}

			~DxgiDeviceLockBase() {
				if (this->deviceManager) {
					if (this->hDevice) {
						if (this->locked) {
							this->deviceManager->UnlockDevice(hDevice, FALSE);
						}
						this->deviceManager->CloseDeviceHandle(hDevice);
					}
				}
			}

			NO_COPY(DxgiDeviceLockBase);

		protected:
			Microsoft::WRL::ComPtr<DeviceManagerT> deviceManager;
			HANDLE hDevice;
			bool locked;
		};


		class MFDXGIDeviceManagerLock : public DxgiDeviceLockBase<IMFDXGIDeviceManager> {
		public:
			MFDXGIDeviceManagerLock(_In_ Microsoft::WRL::ComPtr<IMFDXGIDeviceManager> deviceManager)
				: DxgiDeviceLockBase(deviceManager)
			{}

			template <typename DeviceInterfaceT>
			HRESULT LockDevice(_Out_ DeviceInterfaceT** deviceInterface) {
				if (!this->deviceManager) {
					return S_OK;
				}
				
				HRESULT hr = S_OK;

				if (this->locked) {
					hr = E_FAIL;
				}

				hr = this->deviceManager->LockDevice(this->hDevice, __uuidof(DeviceInterfaceT), (void**)deviceInterface, TRUE);
				if (SUCCEEDED(hr)) {
					this->locked = true;
				}
				return hr;
			}

			HRESULT UnlockDevice() {
				if (!this->deviceManager) {
					return S_OK;
				}

				HRESULT hr = S_OK;

				if (!this->locked) {
					hr = E_FAIL;
				}

				hr = this->deviceManager->UnlockDevice(hDevice, FALSE);
				if (SUCCEEDED(hr)) {
					this->locked = false;
				}
				return hr;
			}
		};
	}
}
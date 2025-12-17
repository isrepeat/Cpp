#pragma once
#include <Helpers/common.h>
#include "DxIncludes.h"
#include <mfobjects.h>
#include <mfapi.h>

namespace HELPERS_NS {
	namespace Dx {
		class MFDXGIDeviceManagerCustom : public Microsoft::WRL::RuntimeClass<
			Microsoft::WRL::RuntimeClassFlags<Microsoft::WRL::RuntimeClassType::WinRtClassicComMix>,
			IMFDXGIDeviceManager>
		{
		public:
			MFDXGIDeviceManagerCustom(Microsoft::WRL::ComPtr<IMFDXGIDeviceManager> mfDxgiDeviceManagerOrig);
			~MFDXGIDeviceManagerCustom() = default;

			HRESULT STDMETHODCALLTYPE CloseDeviceHandle(
				/* [annotation] */
				_In_  HANDLE hDevice) override;

			HRESULT STDMETHODCALLTYPE GetVideoService(
				/* [annotation] */
				_In_  HANDLE hDevice,
				/* [annotation] */
				_In_  REFIID riid,
				/* [annotation] */
				_Outptr_  void** ppService) override;

			HRESULT STDMETHODCALLTYPE LockDevice(
				/* [annotation] */
				_In_  HANDLE hDevice,
				/* [annotation] */
				_In_  REFIID riid,
				/* [annotation] */
				_Outptr_  void** ppUnkDevice,
				/* [annotation] */
				_In_  BOOL fBlock) override;

			HRESULT STDMETHODCALLTYPE OpenDeviceHandle(
				/* [annotation] */
				_Out_  HANDLE* phDevice) override;

			HRESULT STDMETHODCALLTYPE ResetDevice(
				/* [annotation] */
				_In_  IUnknown* pUnkDevice,
				/* [annotation] */
				_In_  UINT resetToken) override;

			HRESULT STDMETHODCALLTYPE TestDevice(
				/* [annotation] */
				_In_  HANDLE hDevice) override;

			HRESULT STDMETHODCALLTYPE UnlockDevice(
				/* [annotation] */
				_In_  HANDLE hDevice,
				/* [annotation] */
				_In_  BOOL fSaveState) override;

		private:
			Microsoft::WRL::ComPtr<IMFDXGIDeviceManager> mfDxgiDeviceManagerOrig;
			Microsoft::WRL::ComPtr<ID3D10Multithread> d3dMultithread;
		};
	}
}
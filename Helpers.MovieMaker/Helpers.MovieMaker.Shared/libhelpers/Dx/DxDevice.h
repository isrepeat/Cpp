#pragma once
#include <Helpers/Dx/DxDevice.h>
#include <Helpers/Mutex.h>

using D2DCtxMt = HH::Dx::D2DCtxMt;
using DxDevice = HH::Dx::DxDeviceSafeObj;
// WORKAROUND: Use mutex type explicitly as EmptyMutex because AvReader hang (need rewrite AvReader logic).
//			   H::Dx::SwapChainPanel also must use such mutex type when create it from MediaRendererWinRT and DxUIRendererWinRt.
using DxDeviceMutex_t = HH::EmptyMutex;
using DxDeviceCtxDynamic = HH::Dx::DxDeviceCtxDynamic;
using DxDeviceCtxSafeObj_t = HH::Dx::DxDeviceCtxSafeObj_t;


//#include "DxDeviceCtx.h"
//#include "DxDeviceMt.h"
//
//#include <Helpers/Dx/DxgiDeviceLock.h>
//#include <Helpers/ThreadSafeObject.hpp>
//#include <Helpers/MultithreadMutex.h>
//#include <Helpers/Thread.h>
//
//#include <mfapi.h>
//#include <mfobjects.h>
//
//#include <d3d11.h>
//#include <d2d1.h>
//#include <d2d1_1.h>
//#include <dwrite.h>
//#include <dxgi.h>
//#include <DirectXMath.h>
//#include <DirectXColors.h>
//#include <wrl.h>
//
//#include <libhelpers\Thread\PPL\critical_section_guard.h>
//
//class MFDXGIDeviceManagerCustom : public Microsoft::WRL::RuntimeClass<
//	Microsoft::WRL::RuntimeClassFlags<Microsoft::WRL::RuntimeClassType::WinRtClassicComMix>,
//	IMFDXGIDeviceManager> 
//{
//public:
//	MFDXGIDeviceManagerCustom(Microsoft::WRL::ComPtr<IMFDXGIDeviceManager> mfDxgiDeviceManagerOrig);
//	~MFDXGIDeviceManagerCustom() = default;
//
//	HRESULT STDMETHODCALLTYPE CloseDeviceHandle(
//		/* [annotation] */
//		_In_  HANDLE hDevice) override;
//
//	HRESULT STDMETHODCALLTYPE GetVideoService(
//		/* [annotation] */
//		_In_  HANDLE hDevice,
//		/* [annotation] */
//		_In_  REFIID riid,
//		/* [annotation] */
//		_Outptr_  void** ppService) override;
//
//	HRESULT STDMETHODCALLTYPE LockDevice(
//		/* [annotation] */
//		_In_  HANDLE hDevice,
//		/* [annotation] */
//		_In_  REFIID riid,
//		/* [annotation] */
//		_Outptr_  void** ppUnkDevice,
//		/* [annotation] */
//		_In_  BOOL fBlock) override;
//
//	HRESULT STDMETHODCALLTYPE OpenDeviceHandle(
//		/* [annotation] */
//		_Out_  HANDLE* phDevice) override;
//
//	HRESULT STDMETHODCALLTYPE ResetDevice(
//		/* [annotation] */
//		_In_  IUnknown* pUnkDevice,
//		/* [annotation] */
//		_In_  UINT resetToken) override;
//
//	HRESULT STDMETHODCALLTYPE TestDevice(
//		/* [annotation] */
//		_In_  HANDLE hDevice) override;
//
//	HRESULT STDMETHODCALLTYPE UnlockDevice(
//		/* [annotation] */
//		_In_  HANDLE hDevice,
//		/* [annotation] */
//		_In_  BOOL fSaveState) override;
//
//private:
//	Microsoft::WRL::ComPtr<IMFDXGIDeviceManager> mfDxgiDeviceManagerOrig;
//	Microsoft::WRL::ComPtr<ID3D10Multithread> d3dMultithread;
//};
//
//
//namespace details {
//	struct DxDeviceParams {
//		static const uint32_t DefaultD3D11CreateFlags;
//
//		uint32_t d3d11CreateFlags;
//
//		DxDeviceParams();
//		DxDeviceParams(uint32_t d3d11CreateFlags);
//	};
//
//	class DxDevice : public DxDeviceMt {
//	public:
//		DxDevice(const DxDeviceParams* params = nullptr);
//		~DxDevice();
//
//		DxDeviceCtxSafeObj_t::_Locked LockContext() const;
//		D3D_FEATURE_LEVEL GetDeviceFeatureLevel() const;
//
//	private:		
//		DxDeviceCtxSafeObj_t ctxSafeObj;
//
//		D3D_FEATURE_LEVEL featureLevel;
//
//		void CreateDeviceIndependentResources();
//		void CreateDeviceDependentResources(const DxDeviceParams* params);
//		void EnableD3DDeviceMultithreading();
//		void CreateD2DDevice();
//		Microsoft::WRL::ComPtr<ID2D1DeviceContext> CreateD2DDeviceContext();
//
//		// Check for SDK Layer support.
//		static bool SdkLayersAvailable();
//	};
//
//	class DxDeviceMF : public DxDevice {
//	public:
//		DxDeviceMF::DxDeviceMF(const DxDeviceParams* params = nullptr);
//		
//		void CreateMFDXGIDeviceManager();
//		Microsoft::WRL::ComPtr<IMFDXGIDeviceManager> GetMFDXGIDeviceManager();
//
//	private:		
//		Microsoft::WRL::ComPtr<MFDXGIDeviceManagerCustom> mfDxgiDeviceManagerCustom;
//	};
//
//
//	class DxVideoDeviceMF : public DxDeviceMF {
//	public:
//		DxVideoDeviceMF();
//	};
//
//
//	class DxDeviceMFLock {
//	public:
//		DxDeviceMFLock(const std::unique_ptr<details::DxDeviceMF>& dxDeviceMf);
//		~DxDeviceMFLock();
//
//	private:
//		HH::Dx::MFDXGIDeviceManagerLock mfDxgiDeviceManagerLock;
//		const std::unique_ptr<details::DxDeviceMF>& dxDeviceMf;
//	};
//}
//
//// TODO: split on two wrapper:
////		 1) Root wrapper - used to lock dxDevice on top of the stack of current thread (it helps avoid deadlocks)
////		 2) Sub wrapper - will be passed down the stack
////using DxDevice = HH::ThreadSafeObjectBaseUniq<HH::MultithreadMutexRecursive, details::DxDevice>;
////using DxVideoDevice = HH::ThreadSafeObjectBaseUniq<HH::MultithreadMutexRecursive, details::DxVideoDevice>;
//
//
//
//// TODO: rename to DxDeviceSafeObj
//using DxDevice = HH::ThreadSafeObject<std::recursive_mutex, std::unique_ptr<details::DxDeviceMF>, details::DxDeviceMFLock>;
//using DxVideoDevice = HH::ThreadSafeObject<std::recursive_mutex, std::unique_ptr<details::DxVideoDeviceMF>, details::DxDeviceMFLock>;
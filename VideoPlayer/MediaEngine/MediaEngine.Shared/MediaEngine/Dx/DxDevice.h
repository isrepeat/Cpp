//#pragma once
//#include "DxDeviceCtx.h"
//#include "DxDeviceMt.h"
//
//#include <Helpers/Dx/DxgiDeviceLock.h>
//#include <Helpers/ThreadSafeObject.hpp>
//#include <Helpers/Thread.h>
//#include <Helpers/Math.h>
//
////#include <Helpers/Dx/DxHelpers.h>
//#include <Helpers/TypeTraits.hpp>
//#include <Helpers/System.h>
//#include <DirectXColors.h>
//#include <DirectXMath.h>
//
//#include <mfapi.h>
//#include <mfobjects.h>
//
//#include <optional>
//#include <dwrite.h>
//#include <dxgi1_3.h>
//#include <d3d11_1.h>
//#include <d3d11_2.h>
//#include <d2d1_2.h>
//#include <d2d1.h>
//#include <wrl.h>
//
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
//namespace Dx {
//	namespace details {
//		struct DxDeviceParams {
//			static const uint32_t DefaultD3D11CreateFlags;
//
//			uint32_t d3d11CreateFlags;
//
//			DxDeviceParams();
//			DxDeviceParams(uint32_t d3d11CreateFlags);
//		};
//
//		class DxDevice : public DxDeviceMt {
//		public:
//			DxDevice(const DxDeviceParams* params = nullptr);
//			~DxDevice();
//
//			DxDeviceCtxSafeObj_t::_Locked LockContext() const;
//			D3D_FEATURE_LEVEL GetDeviceFeatureLevel() const;
//
//		private:
//			DxDeviceCtxSafeObj_t ctxSafeObj;
//
//			D3D_FEATURE_LEVEL featureLevel;
//
//			void CreateDeviceIndependentResources();
//			void CreateDeviceDependentResources(const DxDeviceParams* params);
//			void EnableD3DDeviceMultithreading();
//			void CreateD2DDevice();
//			Microsoft::WRL::ComPtr<ID2D1DeviceContext> CreateD2DDeviceContext();
//
//			// Check for SDK Layer support.
//			static bool SdkLayersAvailable();
//		};
//
//		class DxDeviceMF : public DxDevice {
//		public:
//			DxDeviceMF::DxDeviceMF(const DxDeviceParams* params = nullptr);
//
//			void CreateMFDXGIDeviceManager();
//			Microsoft::WRL::ComPtr<IMFDXGIDeviceManager> GetMFDXGIDeviceManager();
//
//		private:
//			Microsoft::WRL::ComPtr<MFDXGIDeviceManagerCustom> mfDxgiDeviceManagerCustom;
//		};
//
//
//		class DxVideoDeviceMF : public DxDeviceMF {
//		public:
//			DxVideoDeviceMF();
//		};
//
//
//		class DxDeviceMFLock {
//		public:
//			DxDeviceMFLock(const std::unique_ptr<details::DxDeviceMF>& dxDeviceMf);
//			~DxDeviceMFLock();
//
//		private:
//			H::Dx::MFDXGIDeviceManagerLock mfDxgiDeviceManagerLock;
//			const std::unique_ptr<details::DxDeviceMF>& dxDeviceMf;
//		};
//	}
//
//	using DxDeviceSafeObj = H::ThreadSafeObject<std::recursive_mutex, std::unique_ptr<details::DxDeviceMF>, details::DxDeviceMFLock>;
//	//using DxVideoDeviceSafeObj = H::ThreadSafeObject<std::recursive_mutex, std::unique_ptr<details::DxVideoDeviceMF>, details::DxDeviceMFLock>;
//
//
//
//
//	struct Dpi {
//		Dpi()
//			: dpiX{ 0 }
//			, dpiY{ 0 }
//		{}
//
//		Dpi(float dpi)
//			: dpiX{ dpi }
//			, dpiY{ dpi }
//		{}
//
//		Dpi(float dpiX, float dpiY) 
//			: dpiX{ dpiX }
//			, dpiY{ dpiY }
//		{}
//
//		static const Dpi StandardDpi() {
//			static Dpi standardDpi = Dpi(96.0f, 96.0f);
//			return standardDpi;
//		}
//
//		float GetDpiX() const {
//			return this->dpiX;
//		}
//		float GetDpiY() const {
//			return this->dpiY;
//		}
//		float GetDpi() const {
//			assert(static_cast<int>(this->dpiX) == static_cast<int>(this->dpiY));
//			return this->dpiX;
//		}
//
//		float GetScaleFactor() const {
//			return this->GetDpi() / Dpi::StandardDpi().GetDpi();
//		}
//
//	private:
//		const float dpiX;
//		const float dpiY;
//	};
//
//	inline H::Size operator*(const H::Size& size, const Dpi& dpi) {
//		return size * dpi.GetScaleFactor();
//	}
//	//inline H::Size operator/(const H::Size& size, const Dpi& dpi) {
//	//	return size / dpi.GetDpi() / Dpi::StandardDpi().GetDpi();
//	//}
//
//	//struct DisplayInfo {
//	//	static const Dpi GetStandardDpi() {
//	//		static Dpi standardDpi = Dpi(96.0f, 96.0f);
//	//		return standardDpi;
//	//	}
//
//	//	static float GetDpiScaleFactor(const Dpi& currentDpi) {
//	//		return currentDpi.GetDpi() / DisplayInfo::GetStandardDpi().GetDpi();
//	//	}
//	//};
//}
#pragma once
#include <Helpers/common.h>
#include <Helpers/Dx/DxgiDeviceLock.h>
#include <Helpers/ThreadSafeObject.hpp>
#include "DxIncludes.h"

namespace HELPERS_NS {
	namespace Dx {
		class DxDeviceCtx {
		public:
			DxDeviceCtx();
			DxDeviceCtx(
				const Microsoft::WRL::ComPtr<ID2D1DeviceContext>& d2dCtx,
				const Microsoft::WRL::ComPtr<ID3D11DeviceContext3>& d3dCtx);

			DxDeviceCtx(const DxDeviceCtx& other);
			DxDeviceCtx(DxDeviceCtx&& other);
			virtual ~DxDeviceCtx();

			DxDeviceCtx& operator=(const DxDeviceCtx& other);
			DxDeviceCtx& operator=(DxDeviceCtx&& other);

			ID2D1DeviceContext* D2D() const;
			ID3D11DeviceContext3* D3D() const;

		private:
			Microsoft::WRL::ComPtr<ID2D1DeviceContext> d2dCtx;
			Microsoft::WRL::ComPtr<ID3D11DeviceContext3> d3dCtx;
		};


		class DxDeviceCtxLock {
		public:
			DxDeviceCtxLock(const std::unique_ptr<DxDeviceCtx>& dxDeviceCtx);
			~DxDeviceCtxLock();

		private:
			HELPERS_NS::Dx::MFDXGIDeviceManagerLock GetMFDXGIManagerLock(const std::unique_ptr<DxDeviceCtx>& dxDeviceCtx);
			Microsoft::WRL::ComPtr<ID3D10Multithread> GetD3DMultithread(const std::unique_ptr<DxDeviceCtx>& dxDeviceCtx);

		private:
			HELPERS_NS::Dx::MFDXGIDeviceManagerLock mfDxgiDeviceManagerLock;
			Microsoft::WRL::ComPtr<ID3D10Multithread> d3dMultithread;
		};

		using DxDeviceCtxSafeObj_t = HELPERS_NS::ThreadSafeObject<std::recursive_mutex, std::unique_ptr<DxDeviceCtx>, DxDeviceCtxLock>;



		// TODO: need refactoring (try remove const_cast at least)
		class DxDeviceCtxWrapper {
		public:
			DxDeviceCtxWrapper(const DxDeviceCtxSafeObj_t* dxCtxSafeObj)
				: dxCtxSafeObj{ const_cast<DxDeviceCtxSafeObj_t*>(dxCtxSafeObj) }
			{}

			DxDeviceCtxWrapper& operator=(const DxDeviceCtxSafeObj_t* newDxCtxSafeObj) {
				this->dxCtxSafeObj = const_cast<DxDeviceCtxSafeObj_t*>(newDxCtxSafeObj);
				return *this;
			}

			DxDeviceCtxSafeObj_t::_Locked Lock() const {
				assert(this->dxCtxSafeObj);
				return this->dxCtxSafeObj->Lock();
			}

			bool HaveDxCtx() const {
				return dxCtxSafeObj;
			}

		private:
			DxDeviceCtxSafeObj_t* dxCtxSafeObj = nullptr;
		};


		// TODO: need refactoring
		class DxDeviceCtxDynamic {
		public:
			DxDeviceCtxDynamic(const DxDeviceCtxSafeObj_t* dxCtxSafeObj = nullptr)
				: dxDeviceCtxWrapperSafeObj{ DxDeviceCtxWrapper{dxCtxSafeObj} }
			{}

			DxDeviceCtxSafeObj_t::_Locked Lock() const {
				auto dxDeviceCtxWrapperLocked = this->dxDeviceCtxWrapperSafeObj.Lock();
				return dxDeviceCtxWrapperLocked->Lock();
			}

			void SetDxDeviceCtx(const DxDeviceCtxSafeObj_t* newDxCtxSafeObj) {
				assert(newDxCtxSafeObj);
				auto dxDeviceCtxWrapperLocked = this->dxDeviceCtxWrapperSafeObj.Lock();
				if (dxDeviceCtxWrapperLocked->HaveDxCtx()) {
					auto oldDxCtxLocked = dxDeviceCtxWrapperLocked->Lock();
					dxDeviceCtxWrapperLocked.Get() = newDxCtxSafeObj;
				}
				else {
					dxDeviceCtxWrapperLocked.Get() = newDxCtxSafeObj;
				}
			}

		private:
			HELPERS_NS::ThreadSafeObject<std::mutex, DxDeviceCtxWrapper> dxDeviceCtxWrapperSafeObj;
		};
	}
}
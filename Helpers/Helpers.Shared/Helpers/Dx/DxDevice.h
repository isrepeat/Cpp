#pragma once
#include <Helpers/common.h>
#include "MFDXGIManagerCustom.h"
#include "DxgiDeviceLock.h"
#include "DxDeviceCtx.h"
#include "DxDeviceMt.h"
#include "DxIncludes.h"
#include "DxHelpers.h"

#include <Helpers/ThreadSafeObject.hpp>
#include <Helpers/Com/ComMutex.h>
#include <Helpers/Passkey.hpp>
#include <Helpers/Thread.h>
#include <Helpers/System.h>
#include <Helpers/Math.h>
#include <DirectXColors.h>
#include <DirectXMath.h>

#include <optional>
#include <mfobjects.h>
#include <mfapi.h>

namespace HELPERS_NS {
	namespace Dx {
		class SwapChainPanel;
		class ISwapChainPanel;

		namespace details {
			struct DxDeviceParams {
				static const uint32_t DefaultD3D11CreateFlags;
				uint32_t d3d11CreateFlags;

				DxDeviceParams();
				DxDeviceParams(uint32_t d3d11CreateFlags);
			};


			class DxDevice : public DxDeviceMt {
			public:
				DxDevice(const std::optional<DxDeviceParams>& params = std::nullopt);
				virtual ~DxDevice();

				DxDeviceCtxSafeObj_t::_Locked LockContext() const;
				D3D_FEATURE_LEVEL GetDeviceFeatureLevel() const;
				
				void CreateDxgiFactory();

				void SetAssociatedSwapChainPanel(
					Passkey<HELPERS_NS::Dx::SwapChainPanel*>,
					Microsoft::WRL::ComPtr<HELPERS_NS::Dx::ISwapChainPanel> associatedSwapChainPanel);
				
				Microsoft::WRL::ComPtr<HELPERS_NS::Dx::ISwapChainPanel> GetAssociatedSwapChainPanel() const;

			private:
				void CreateDeviceIndependentResources();
				void CreateDeviceDependentResources(const std::optional<DxDeviceParams>& params);
				void EnableD3DDeviceMultithreading();
				void CreateD2DDevice();
				Microsoft::WRL::ComPtr<ID2D1DeviceContext> CreateD2DDeviceContext();
			
			private:
				DxDeviceCtxSafeObj_t ctxSafeObj;
				D3D_FEATURE_LEVEL featureLevel;
				Microsoft::WRL::ComPtr<HELPERS_NS::Dx::ISwapChainPanel> associatedSwapChainPanel;
			};


			class DxDeviceMF : public DxDevice {
			public:
				DxDeviceMF(const std::optional<DxDeviceParams>& params = std::nullopt);
				
				void CreateMFDXGIDeviceManager();
				Microsoft::WRL::ComPtr<IMFDXGIDeviceManager> GetMFDXGIDeviceManager();

			private:
				Microsoft::WRL::ComPtr<MFDXGIDeviceManagerCustom> mfDxgiDeviceManagerCustom;
			};


			class DxVideoDeviceMF : public DxDeviceMF {
			public:
				DxVideoDeviceMF();
			};

			

			class DxDeviceMFLock {
			public:
				DxDeviceMFLock(const std::unique_ptr<details::DxDeviceMF>& dxDeviceMf);
				~DxDeviceMFLock();

			private:
				HELPERS_NS::Dx::MFDXGIDeviceManagerLock mfDxgiDeviceManagerLock;
				const std::unique_ptr<details::DxDeviceMF>& dxDeviceMf;
			};
		}


		// TODO: Write custom DxDeviceSafeObj to be compatible between winRt dll's.
		//       Now only mutex is safe because it is called via virtual methods,
		//       but you also need make safe other dxDevice methods.		
		using DxDeviceSafeObj = HELPERS_NS::ThreadSafeObject<HELPERS_NS::Com::Mutex, std::unique_ptr<details::DxDevice>>;
		using DxDeviceMFSafeObj = HELPERS_NS::ThreadSafeObject<HELPERS_NS::Com::Mutex, std::unique_ptr<details::DxDeviceMF>>;

		
		//class DxDeviceSafeObj;
		//struct DxDeviceLockedObjBase
		//{
		//	using CreatorT = DxDeviceSafeObj;
		//	using MutexT = HELPERS_NS::Com::Mutex<std::recursive_mutex>;

		//	DxDeviceLockedObjBase(MutexT& mx, CreatorT* creator, int& ownerThreadId)
		//		: lk{ mx }
		//		, creator{ creator }
		//	{
		//		ownerThreadId = ::GetCurrentThreadId();
		//	}

		//	~DxDeviceLockedObjBase() {
		//	}

		//	CreatorT* GetCreator() {
		//		return this->creator;
		//	}

		//private:
		//	std::unique_lock<MutexT> lk;
		//	CreatorT* creator;
		//};


		////struct IDxDeviceLockedObj {
		////	virtual ~IDxDeviceLockedObj();
		////};

		//struct DxDeviceLockedObj : DxDeviceLockedObjBase
		//{
		//	using ObjT = details::DxDeviceMF;

		//	DxDeviceLockedObj(
		//		MutexT& mx,
		//		std::unique_ptr<ObjT>& obj,
		//		CreatorT* creator,
		//		int& ownerThreadId)
		//		: DxDeviceLockedObjBase(mx, creator, ownerThreadId)
		//		, obj{ obj } // [by design]
		//	{}

		//	~DxDeviceLockedObj() {
		//	}

		//	std::unique_ptr<ObjT>& operator->() {
		//		return this->obj;
		//	}
		//	std::unique_ptr<ObjT>& Get() {
		//		return this->obj;
		//	}

		//private:
		//	std::unique_ptr<ObjT>& obj;
		//};



		//class IDxDeviceSafeObj {
		//public:
		//	using _Locked = DxDeviceLockedObj;
		//	virtual _Locked Lock() = 0;
		//};

		//class DxDeviceSafeObj : public IDxDeviceSafeObj {
		//public:
		//	using MutexT = HELPERS_NS::Com::Mutex<std::recursive_mutex>;
		//	using ObjT = details::DxDeviceMF;

		//	DxDeviceSafeObj()
		//		: obj{ std::make_unique<ObjT>() }
		//	{}

		//	~DxDeviceSafeObj() {
		//		int xx = 9;
		//	}

		//	template <typename... Args>
		//	DxDeviceSafeObj(Args&&... args)
		//		: obj{ std::make_unique<ObjT>(std::forward<Args>(args)...) }
		//	{}

		//	DxDeviceSafeObj(DxDeviceSafeObj& other) = delete;
		//	DxDeviceSafeObj& operator=(DxDeviceSafeObj& other) = delete;

		//	template <typename OtherT, std::enable_if_t<std::is_convertible_v<std::unique_ptr<OtherT>, std::unique_ptr<ObjT>>, int> = 0>
		//	DxDeviceSafeObj(std::unique_ptr<OtherT>&& otherObj)
		//		: obj{ std::move(otherObj) }
		//	{}

		//	template <typename OtherT, std::enable_if_t<std::is_convertible_v<std::unique_ptr<OtherT>, std::unique_ptr<ObjT>>, int> = 0>
		//	DxDeviceSafeObj& operator=(std::unique_ptr<OtherT>&& otherObj) {
		//		if (this->obj.get() != otherObj.get()) {
		//			this->obj = std::move(otherObj);
		//		}
		//		return *this;
		//	}

		//	_Locked Lock() override {
		//		return _Locked{ this->mx, this->obj, this, this->ownerThreadId };
		//	}

		//private:
		//	MutexT mx;
		//	int ownerThreadId = -1;
		//	std::unique_ptr<ObjT> obj;
		//};
	}
}
#pragma once
#include <Helpers/MultithreadMutex.h>
#include "DxIncludes.h"
#include "D2DCtxMt.h"

namespace HELPERS_NS {
	namespace Dx {
		namespace details {
			// directx interfaces that can be used in multithreaded mode
			class DxDeviceMt {
			public:
				Microsoft::WRL::ComPtr<ID2D1Device> GetD2DDevice() const;
				Microsoft::WRL::ComPtr<ID2D1Factory3> GetD2DFactory() const;
				Microsoft::WRL::ComPtr<IDWriteFactory3> GetDwriteFactory() const;
				Microsoft::WRL::ComPtr<IWICImagingFactory2> GetWICFactory() const;

				Microsoft::WRL::ComPtr<IDXGIFactory2> GetDxgiFactory() const;

				Microsoft::WRL::ComPtr<ID3D11Device3> GetD3DDevice() const;
				Microsoft::WRL::ComPtr<ID3D10Multithread> GetD3DMultithread() const;

				HELPERS_NS::Dx::D2DCtxMt* GetD2DCtxMt();

			protected:
				Microsoft::WRL::ComPtr<ID2D1Device> d2dDevice;
				Microsoft::WRL::ComPtr<ID2D1Factory3> d2dFactory;
				Microsoft::WRL::ComPtr<IDWriteFactory3> dwriteFactory;
				Microsoft::WRL::ComPtr<IWICImagingFactory2> wicFactory;

				Microsoft::WRL::ComPtr<IDXGIFactory2> dxgiFactory;

				Microsoft::WRL::ComPtr<ID3D11Device3> d3dDevice;
				Microsoft::WRL::ComPtr<ID3D10Multithread> d3dMultithread;

				HELPERS_NS::Dx::D2DCtxMt d2dCtxMt;
			};
		}
	}
}
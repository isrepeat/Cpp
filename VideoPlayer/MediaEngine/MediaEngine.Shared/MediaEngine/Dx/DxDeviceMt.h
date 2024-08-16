#pragma once
#include <Helpers/MultithreadMutex.h>

#include <d3d11.h>
#include <d2d1.h>
#include <d2d1_1.h>
#include <dwrite.h>
#include <wrl.h>

namespace Dx {
	namespace details {
		// directx interfaces that can be used in multithreaded mode
		class DxDeviceMt {
		public:
			Microsoft::WRL::ComPtr<ID2D1Device> GetD2DDevice() const;
			Microsoft::WRL::ComPtr<ID2D1Factory1> GetD2DFactory() const;
			Microsoft::WRL::ComPtr<IDWriteFactory> GetDwriteFactory() const;

			Microsoft::WRL::ComPtr<ID3D11Device> GetD3DDevice() const;
			Microsoft::WRL::ComPtr<ID3D10Multithread> GetD3DMultithread() const;

		protected:
			Microsoft::WRL::ComPtr<ID2D1Device> d2dDevice;
			Microsoft::WRL::ComPtr<ID2D1Factory1> d2dFactory;
			Microsoft::WRL::ComPtr<IDWriteFactory> dwriteFactory;

			Microsoft::WRL::ComPtr<ID3D11Device> d3dDevice;
			Microsoft::WRL::ComPtr<ID3D10Multithread> d3dMultithread;
		};
	}
}
#pragma once
#include <d3d11.h>
#include <d3d9.h>
#include <d2d1.h>
#include <d2d1_1.h>
#include <dwrite.h>
#include <dxgi.h>
#include "DxDeviceCtx.h"
#include <wrl.h>
#include <stdint.h>
#include <memory>


class DxDevice {
public:
	DxDevice();
	~DxDevice();

	std::shared_ptr<DxDeviceCtx> GetContext() const;

	D3D_FEATURE_LEVEL GetDeviceFeatureLevel() const;

	Microsoft::WRL::ComPtr<IDWriteFactory> GetDwriteFactory() const;
	Microsoft::WRL::ComPtr<ID2D1Factory1> GetD2DFactory() const;
	Microsoft::WRL::ComPtr<ID3D11Device> GetD3DDevice() const;
	Microsoft::WRL::ComPtr<ID2D1Device> GetD2DDevice() const;

	Microsoft::WRL::ComPtr<IDXGISwapChain1> CreateSwapChainForHwnd(DXGI_SWAP_CHAIN_DESC1 swapChainDesc, HWND hwnd);

private:
	std::shared_ptr<DxDeviceCtx> ctx;

	D3D_FEATURE_LEVEL featureLevel;

	void CreateDeviceIndependentResources();
	void CreateDeviceDependentResources();
	void CreateD2DDevice();
	Microsoft::WRL::ComPtr<ID2D1DeviceContext> CreateD2DDeviceContext();

private:
	Microsoft::WRL::ComPtr<IDWriteFactory> dwriteFactory;
	Microsoft::WRL::ComPtr<ID2D1Factory1> d2dFactory;
	Microsoft::WRL::ComPtr<ID3D11Device> d3dDevice;
	Microsoft::WRL::ComPtr<ID2D1Device> d2dDevice;
};

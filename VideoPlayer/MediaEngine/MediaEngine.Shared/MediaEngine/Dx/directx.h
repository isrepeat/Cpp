#pragma once
#include <Helpers/System.h>
#include <Helpers/Math.h>
#include <DirectXColors.h>
#include <DirectXMath.h>

#include <Helpers/Dx/DxDevice.h>

#include <windows.h>
#include <mfobjects.h>
#include <functional>
#include <d2d1effects.h>
#include <dxgi1_3.h>
#include <d3d11_1.h>
#include <d3d11_2.h>
#include <d2d1_2.h>
#include <d2d1.h>
#include <wrl.h>

HRESULT CreateSwapChain(Microsoft::WRL::ComPtr<ID3D11Device> d3dDevice, IDXGISwapChain1** pDXGISwapChain);
HRESULT ResizeSwapChain(Microsoft::WRL::ComPtr<IDXGISwapChain1>& pDXGISwapChain, H::Size size);

namespace DxTools {
	// Create a ID2D1Bitmap1 from the frame texture. Returns a nullptr on fail.
	Microsoft::WRL::ComPtr<ID2D1Bitmap1> CreateFrameBitmap(
		H::Dx::DxDeviceCtxSafeObj_t::_Locked& dxCtx,
		Microsoft::WRL::ComPtr<ID3D11Texture2D> pTexture);

	Microsoft::WRL::ComPtr<ID3D11Texture2D> RenderTextToTexture(
		H::Dx::DxDeviceSafeObj::_Locked& dxDev,
		const std::wstring& text,
		const D3D11_TEXTURE2D_DESC& texDesc,
		int fontSize = 42);

	Microsoft::WRL::ComPtr<ID3D11Texture2D> RenderTextToTexture(
		H::Dx::DxDeviceSafeObj::_Locked& dxDev,
		const std::wstring& text,
		H::Size texSize,
		int fontSize = 42);
}
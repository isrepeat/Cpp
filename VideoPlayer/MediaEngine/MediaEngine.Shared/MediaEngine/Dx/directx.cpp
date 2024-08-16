#include "directx.h"
#include <Helpers/Dx/DxgiDeviceLock.h>

#pragma comment(lib, "D3D11.lib")
#pragma comment(lib, "D2D1.lib")
#pragma comment(lib, "dxguid.lib")


namespace {
	DXGI_SWAP_CHAIN_DESC1 CreateSwapChainDesc(UINT width, UINT height) {
		DXGI_SWAP_CHAIN_DESC1 swapChainDesc = { 0 };

		swapChainDesc.Width = std::max<UINT>(width, 10);
		swapChainDesc.Height = std::max<UINT>(height, 10);
		swapChainDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		swapChainDesc.Stereo = false;
		swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;

		// Don't use Multi-sampling
		swapChainDesc.SampleDesc.Count = 1;
		swapChainDesc.SampleDesc.Quality = 0;

		// Use more than 1 buffer to enable Flip effect.
		//swapChainDesc.BufferCount = 4;
		swapChainDesc.BufferCount = 2;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;// | DXGI_USAGE_BACK_BUFFER;
		swapChainDesc.Scaling = DXGI_SCALING_NONE;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;

		return swapChainDesc;
	}
}

HRESULT CreateSwapChain(Microsoft::WRL::ComPtr<ID3D11Device> d3dDevice, IDXGISwapChain1** ppDXGISwapChain1) {
	HRESULT hr = S_OK;

	Microsoft::WRL::ComPtr<IDXGIDevice1> dxgiDevice1;
	hr = d3dDevice->QueryInterface(IID_PPV_ARGS(dxgiDevice1.GetAddressOf()));
	H::System::ThrowIfFailed(hr);

	// Ensure that DXGI does not queue more than one frame at a time.
	hr = dxgiDevice1->SetMaximumFrameLatency(1);
	H::System::ThrowIfFailed(hr);

	Microsoft::WRL::ComPtr<IDXGIAdapter> dxgiAdapter;
	hr = dxgiDevice1->GetParent(IID_PPV_ARGS(dxgiAdapter.GetAddressOf()));
	H::System::ThrowIfFailed(hr);

	Microsoft::WRL::ComPtr<IDXGIFactory2> dxgiFactory;
	hr = dxgiAdapter->GetParent(IID_PPV_ARGS(dxgiFactory.GetAddressOf()));
	H::System::ThrowIfFailed(hr);

	auto swapChainDesc = CreateSwapChainDesc(100, 100);
	swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
	swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT;

	hr = dxgiFactory->CreateSwapChainForComposition(d3dDevice.Get(), &swapChainDesc, nullptr, ppDXGISwapChain1);
	H::System::ThrowIfFailed(hr);
	return hr;
}


HRESULT ResizeSwapChain(Microsoft::WRL::ComPtr<IDXGISwapChain1>& pDXGISwapChain, H::Size size) {
	return pDXGISwapChain->ResizeBuffers(
		2, // Double-buffered swap chain.
		//0,
		size.width,
		size.height,
		DXGI_FORMAT_B8G8R8A8_UNORM, //DXGI_FORMAT_UNKNOWN,
		DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT // Enable GetFrameLatencyWaitableObject()
		//0
	);
}




namespace DxTools {
	Microsoft::WRL::ComPtr<ID2D1Bitmap1> CreateFrameBitmap(
		H::Dx::DxDeviceCtxSafeObj_t::_Locked& dxCtx,
		Microsoft::WRL::ComPtr<ID3D11Texture2D> texture)
	{
		Microsoft::WRL::ComPtr<IDXGISurface> dxgiSurface;
		HRESULT hr = texture.As(&dxgiSurface);
		if (FAILED(hr))
			return nullptr;

		D3D11_TEXTURE2D_DESC desc;
		texture->GetDesc(&desc);

		D2D1_BITMAP_PROPERTIES1 bitmapProperties =
			D2D1::BitmapProperties1(
				D2D1_BITMAP_OPTIONS_NONE,
				D2D1::PixelFormat(desc.Format, D2D1_ALPHA_MODE_IGNORE)
			);

		Microsoft::WRL::ComPtr<ID2D1Bitmap1> pBitmap;
		hr = dxCtx->D2D()->CreateBitmapFromDxgiSurface(
			dxgiSurface.Get(),
			&bitmapProperties,
			&pBitmap
		);

		if (FAILED(hr))
			return nullptr;

		return pBitmap;
	}

	Microsoft::WRL::ComPtr<ID3D11Texture2D> RenderTextToTexture(
		H::Dx::DxDeviceSafeObj::_Locked& dxDev,
		const std::wstring& text,
		const D3D11_TEXTURE2D_DESC& texDesc,
		int fontSize)
	{
		HRESULT hr = S_OK;
		auto dxCtx = dxDev->LockContext();

		Microsoft::WRL::ComPtr<ID3D11Texture2D> tex;
		hr = dxDev->GetD3DDevice()->CreateTexture2D(&texDesc, nullptr, tex.GetAddressOf());
		H::System::ThrowIfFailed(hr);

		Microsoft::WRL::ComPtr<IDXGISurface2> surface;
		hr = tex.As(&surface);
		H::System::ThrowIfFailed(hr);

		D2D1_BITMAP_PROPERTIES1 bitmapProperties =
			D2D1::BitmapProperties1(
				D2D1_BITMAP_OPTIONS_TARGET,
				D2D1::PixelFormat(texDesc.Format, D2D1_ALPHA_MODE_PREMULTIPLIED)
			);

		Microsoft::WRL::ComPtr<ID2D1Bitmap1> bitmap;
		hr = dxCtx->D2D()->CreateBitmapFromDxgiSurface(surface.Get(), &bitmapProperties, bitmap.GetAddressOf());
		H::System::ThrowIfFailed(hr);

		dxCtx->D2D()->SetTarget(bitmap.Get());

		Microsoft::WRL::ComPtr<IDWriteTextFormat> textFormat;
		hr = dxDev->GetDwriteFactory()->CreateTextFormat(L"Segoe UI", nullptr, DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, fontSize, L"en-US", textFormat.GetAddressOf());
		H::System::ThrowIfFailed(hr);

		Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> blackBrush;
		hr = dxCtx->D2D()->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black), blackBrush.GetAddressOf());
		H::System::ThrowIfFailed(hr);

		dxCtx->D2D()->BeginDraw();
		dxCtx->D2D()->SetTransform(D2D1::Matrix3x2F::Identity());
		dxCtx->D2D()->Clear(D2D1::ColorF(D2D1::ColorF::Gray));

		dxCtx->D2D()->DrawText(
			text.c_str(),
			text.size(),
			textFormat.Get(),
			D2D1::RectF(0, 0, texDesc.Width, texDesc.Height),
			blackBrush.Get()
		);

		hr = dxCtx->D2D()->EndDraw();
		H::System::ThrowIfFailed(hr);
		return tex;
	}

	Microsoft::WRL::ComPtr<ID3D11Texture2D> RenderTextToTexture(
		H::Dx::DxDeviceSafeObj::_Locked& dxDev,
		const std::wstring& text,
		H::Size texSize,
		int fontSize)
	{
		// NOTE: If you want fill D3D11_TEXTURE2D_DESC manually ensure that all field is set or 
		//       have default (non zero) values, because by default struct have stack trash.
		D3D11_TEXTURE2D_DESC texDesc = CD3D11_TEXTURE2D_DESC(
			DXGI_FORMAT_B8G8R8A8_UNORM,
			texSize.width, texSize.height,
			1, 1, D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE);

		return RenderTextToTexture(dxDev, text, texDesc, fontSize);
	}
}
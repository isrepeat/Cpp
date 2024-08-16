#include "Direct2DRenderer.h"
#include <Helpers/System.h>

Direct2DRenderer::Direct2DRenderer() {
}

void Direct2DRenderer::CreateSwapChain(HWND hwnd) {
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = { 0 };
	swapChainDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = 2;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_SEQUENTIAL;

	swapChain = dxDev.CreateSwapChainForHwnd(swapChainDesc, hwnd);

	CreateBitmapFromSwapchain();

	dxDev.GetContext()->D2D()->SetTarget(d2d1RenderTargetBitmap.Get());
}

void Direct2DRenderer::Resize(Size newSize) {
	if (swapChain != nullptr) {
		dxDev.GetContext()->D2D()->SetTarget(nullptr);
		d2d1RenderTargetBitmap->Release();

		// We must release all refs on swapChain->GetBuffer(...) before ResizeBuffers(...)
		auto hr = swapChain->ResizeBuffers(0, newSize.width, newSize.height, DXGI_FORMAT_UNKNOWN, 0);
		H::System::ThrowIfFailed(hr);

		CreateBitmapFromSwapchain();
		dxDev.GetContext()->D2D()->SetTarget(d2d1RenderTargetBitmap.Get());
	}
}


void Direct2DRenderer::Render() {
	//dxDev.GetContext()->D2D()->BeginDraw();
	//dxDev.GetContext()->D2D()->Clear(D2D1::ColorF(D2D1::ColorF::Gray));

	this->Draw(dxDev);
	//d2d1DeviceContext->DrawImage(rotateEffect.Get());

	//dxDev.GetContext()->D2D()->EndDraw();
	swapChain->Present(1, 0);
}


void Direct2DRenderer::CreateBitmapFromSwapchain() {
	Microsoft::WRL::ComPtr<IDXGISurface> dxgiBackBuffer;

	HRESULT hr = swapChain->GetBuffer(0, IID_PPV_ARGS(&dxgiBackBuffer));
	H::System::ThrowIfFailed(hr);

	D2D1_BITMAP_PROPERTIES1 bitmapProperties = D2D1::BitmapProperties1(
		D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW, 
		D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED));

	// Get a D2D surface from the DXGI back buffer to use as the D2D render target.
	H::System::ThrowIfFailed(dxDev.GetContext()->D2D()->CreateBitmapFromDxgiSurface(dxgiBackBuffer.Get(), &bitmapProperties, d2d1RenderTargetBitmap.GetAddressOf()));
}

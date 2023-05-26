#pragma once
#include "DxDevice.h"
#include "Size.h"

class Direct2DRenderer {
public:
	Direct2DRenderer();

	void CreateSwapChain(HWND hwnd);
	void Resize(Size newSize);

	void Render();
	virtual void Draw(DxDevice& dxDev) = 0;

private:
	void CreateBitmapFromSwapchain();

private:
	DxDevice dxDev;
	Microsoft::WRL::ComPtr<IDXGISwapChain1> swapChain;
	Microsoft::WRL::ComPtr<ID2D1BitmapRenderTarget> d2d1RenderTarget;
	Microsoft::WRL::ComPtr<ID2D1Bitmap1> d2d1RenderTargetBitmap;
};
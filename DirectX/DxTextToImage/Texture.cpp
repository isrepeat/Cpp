#include "Texture.h"
#include <Helpers/System.h>


std::vector<uint8_t> Texture::RenderTextToBytesArray(const std::wstring& text, H::Size texSize, int fontSize) {
	HRESULT hr = S_OK;

	DxDevice dxDev;

	// If you want fill D3D11_TEXTURE2D_DESC manually ensure that all field is set or 
	// have default (non zero) values, because by default struct have stack trash.
	D3D11_TEXTURE2D_DESC texDesc = CD3D11_TEXTURE2D_DESC(
		DXGI_FORMAT_B8G8R8A8_UNORM,
		texSize.width, texSize.height,
		1, 1, D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE);


	Microsoft::WRL::ComPtr<ID3D11Texture2D> tex;
	hr = dxDev.GetD3DDevice()->CreateTexture2D(&texDesc, nullptr, tex.GetAddressOf());
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
	hr = dxDev.GetContext()->D2D()->CreateBitmapFromDxgiSurface(surface.Get(), &bitmapProperties, bitmap.GetAddressOf());
	H::System::ThrowIfFailed(hr);

	dxDev.GetContext()->D2D()->SetTarget(bitmap.Get());

	Microsoft::WRL::ComPtr<IDWriteTextFormat> textFormat;
	hr = dxDev.GetDwriteFactory()->CreateTextFormat(L"Segoe UI", nullptr, DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, fontSize, L"en-US", textFormat.GetAddressOf());
	H::System::ThrowIfFailed(hr);

	Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> blackBrush;
	hr = dxDev.GetContext()->D2D()->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black), blackBrush.GetAddressOf());
	H::System::ThrowIfFailed(hr);


	dxDev.GetContext()->D2D()->BeginDraw();
	dxDev.GetContext()->D2D()->SetTransform(D2D1::Matrix3x2F::Identity());
	dxDev.GetContext()->D2D()->Clear(D2D1::ColorF(D2D1::ColorF::Gray));

	//dxDev.GetContext()->D2D()->DrawRectangle(D2D1::RectF(0, 0, 400, 400), blackBrush.Get());

	dxDev.GetContext()->D2D()->DrawText(
		text.c_str(),
		text.size(),
		textFormat.Get(),
		D2D1::RectF(0, 0, texDesc.Width, texDesc.Height),
		blackBrush.Get()
	);
	dxDev.GetContext()->D2D()->EndDraw();

	auto res = Texture::GetDataAsVector(dxDev.GetContext()->D2D().Get());
	return res;
}


std::vector<uint8_t> Texture::GetDataAsVector(ID2D1DeviceContext* d2dCtx) {
    HRESULT hr = S_OK;
    Microsoft::WRL::ComPtr<ID2D1Bitmap1> copyBitmap;

    Microsoft::WRL::ComPtr<ID2D1Image> targetImage;
    d2dCtx->GetTarget(targetImage.GetAddressOf());

    Microsoft::WRL::ComPtr<ID2D1Bitmap1> targetBitmap;
    hr = targetImage.As(&targetBitmap);
    H::System::ThrowIfFailed(hr);

    D2D1_BITMAP_PROPERTIES1 prop;
    targetBitmap->GetDpi(&prop.dpiX, &prop.dpiY);
    prop.pixelFormat = targetBitmap->GetPixelFormat();
    prop.bitmapOptions = D2D1_BITMAP_OPTIONS_CPU_READ | D2D1_BITMAP_OPTIONS_CANNOT_DRAW;
    Microsoft::WRL::ComPtr<ID2D1ColorContext> colorContext;
    targetBitmap->GetColorContext(colorContext.GetAddressOf());
    prop.colorContext = colorContext.Get();

    auto textureSize = targetBitmap->GetSize();
    auto texSizeU = D2D1::SizeU((uint32_t)textureSize.width, (uint32_t)textureSize.height);

    hr = d2dCtx->CreateBitmap(texSizeU, nullptr, 0, prop, copyBitmap.GetAddressOf());
    H::System::ThrowIfFailed(hr);

    hr = copyBitmap->CopyFromBitmap(nullptr, targetBitmap.Get(), nullptr);
    H::System::ThrowIfFailed(hr);

    D2D1_MAPPED_RECT mappedData;
    hr = copyBitmap->Map(D2D1_MAP_OPTIONS::D2D1_MAP_OPTIONS_READ, &mappedData);
    H::System::ThrowIfFailed(hr);

    std::vector<uint8_t> res;
    res.resize(texSizeU.width * texSizeU.height * 4);

    if (mappedData.pitch == texSizeU.width * 4) {
        std::memcpy(res.data(), mappedData.bits, res.size());
    }
    else {
        const uint8_t* src = (const uint8_t*)mappedData.bits;
        uint8_t* dst = res.data();

        for (uint32_t y = 0; y < texSizeU.height; y++, src += mappedData.pitch, dst += texSizeU.width * 4) {
            std::memcpy(dst, src, texSizeU.width * 4);
        }
    }

    hr = copyBitmap->Unmap();
    H::System::ThrowIfFailed(hr);

    return res;
}
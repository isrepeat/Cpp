#include "Drawer2D.h"
#include <Helpers/System.h>

Drawer2D::Drawer2D() {
}

void Drawer2D::Draw(DxDevice& dxDev) {
    if (!blackBrush) {
        H::System::ThrowIfFailed(dxDev.GetContext()->D2D()->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black), blackBrush.GetAddressOf()));
    }

    if (!textFormat) {
        H::System::ThrowIfFailed(dxDev.GetDwriteFactory()->CreateTextFormat(L"Segoe UI", nullptr, DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, 42.0f, L"en-US", textFormat.GetAddressOf()));
    }

    static const WCHAR sc_helloWorld[] = L"Hello, World!";

    // Retrieve the size of the render target.
    D2D1_SIZE_F renderTargetSize = dxDev.GetContext()->D2D()->GetSize();

    dxDev.GetContext()->D2D()->BeginDraw();
    dxDev.GetContext()->D2D()->SetTransform(D2D1::Matrix3x2F::Identity());
    dxDev.GetContext()->D2D()->Clear(D2D1::ColorF(D2D1::ColorF::Gray));

    //dxDev.GetContext()->D2D()->DrawRectangle(D2D1::RectF(0, 0, renderTargetSize.width, renderTargetSize.height), blackBrush.Get());
    dxDev.GetContext()->D2D()->DrawRectangle(D2D1::RectF(0, 0, 400, 400), blackBrush.Get());

    dxDev.GetContext()->D2D()->DrawTextW(
        sc_helloWorld,
        ARRAYSIZE(sc_helloWorld) - 1,
        textFormat.Get(),
        D2D1::RectF(0, 0, renderTargetSize.width, renderTargetSize.height),
        blackBrush.Get()
    );

    dxDev.GetContext()->D2D()->EndDraw();
}

void Drawer2D::Resize(Size newSize) {
    Direct2DRenderer::Resize(newSize);
}

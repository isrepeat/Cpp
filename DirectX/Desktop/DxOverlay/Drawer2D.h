#pragma once
#include "Direct2DRenderer.h"

class Drawer2D : public Direct2DRenderer {
public:
	Drawer2D();

	void Draw(DxDevice& dxDev) override;
	void Resize(Size newSize);

private:
	Microsoft::WRL::ComPtr<IDWriteTextFormat> textFormat;
	Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> blackBrush;
};
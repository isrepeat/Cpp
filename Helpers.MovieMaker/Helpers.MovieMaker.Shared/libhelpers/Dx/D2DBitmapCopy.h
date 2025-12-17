#pragma once
#include "DxDevice.h"

class D2DBitmapCopy {
public:
    void operator()(
        ID2D1DeviceContext* ctx,
        const D2D1_RECT_F& dst, ID2D1Bitmap* dstBmp,
        const D2D1_RECT_F& src, ID2D1Bitmap* srcBmp);

private:
    void CheckBmpForCopy(ID2D1DeviceContext* ctx, const D2D1_SIZE_U& srcRectSize, ID2D1Bitmap* srcBmp);

    static D2D1_RECT_F MakeRect(const D2D1_SIZE_U& size);
    static D2D1_RECT_F Intersect(const D2D1_RECT_F& a, const D2D1_RECT_F& b);
    static D2D1_SIZE_F Size(const D2D1_RECT_F& r);
    static D2D1_SIZE_U Size(const D2D1_RECT_U& r);
    static D2D1_RECT_F Round(const D2D1_RECT_F& r);

    Microsoft::WRL::ComPtr<ID2D1Bitmap> bmpForCopy;
};

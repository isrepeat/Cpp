#include "pch.h"
#include "D2DBitmapCopy.h"

#include <cassert>
#include <algorithm>
#include <libhelpers\HSystem.h>

void D2DBitmapCopy::operator()(
    ID2D1DeviceContext* ctx,
    const D2D1_RECT_F& dst, ID2D1Bitmap* dstBmp,
    const D2D1_RECT_F& src, ID2D1Bitmap* srcBmp)
{
    if (!ctx || !dstBmp || !srcBmp) {
        assert(false);
        return;
    }

    if (dstBmp->GetPixelFormat().format != srcBmp->GetPixelFormat().format) {
        assert(false);
        return;
    }

    auto dstBmpRect = D2DBitmapCopy::MakeRect(dstBmp->GetPixelSize());
    auto srcBmpRect = D2DBitmapCopy::MakeRect(srcBmp->GetPixelSize());

    auto dstRect = D2DBitmapCopy::Intersect(dstBmpRect, dst);
    auto srcRect = D2DBitmapCopy::Intersect(srcBmpRect, D2DBitmapCopy::Round(src));

    auto dstSize = D2DBitmapCopy::Size(dstRect);
    auto srcSize = D2DBitmapCopy::Size(srcRect);

    if (dstSize.width <= 0.f || dstSize.height <= 0.f || srcSize.width <= 0.f || srcSize.height <= 0.f) {
        // dst or src rect out of dstBmp or srcBmp
        return;
    }

    // after Round+Intersect it's safe to cast float to uint32_t
    auto srcRectU = D2D1::RectU(
        static_cast<uint32_t>(srcRect.left),
        static_cast<uint32_t>(srcRect.top),
        static_cast<uint32_t>(srcRect.right),
        static_cast<uint32_t>(srcRect.bottom)
    );

    this->CheckBmpForCopy(ctx, D2DBitmapCopy::Size(srcRectU), srcBmp);

    HRESULT hr = S_OK;

    hr = this->bmpForCopy->CopyFromBitmap(nullptr, srcBmp, &srcRectU);
    H::System::ThrowIfFailed(hr);

    auto bmpForCopyScale = D2D1::SizeF(
        dstSize.width / srcSize.width,
        dstSize.height / srcSize.height
    );

    ctx->SetTransform(
        D2D1::Matrix3x2F::Scale(bmpForCopyScale) *
        D2D1::Matrix3x2F::Translation(dstRect.left, dstRect.top)
    );

    ctx->DrawImage(this->bmpForCopy.Get());
}

void D2DBitmapCopy::CheckBmpForCopy(ID2D1DeviceContext* ctx, const D2D1_SIZE_U& srcRectSize, ID2D1Bitmap* srcBmp) {
    if (this->bmpForCopy) {
        auto bmpForCopySize = this->bmpForCopy->GetPixelSize();

        if (bmpForCopySize == srcRectSize &&
            this->bmpForCopy->GetPixelFormat().format == srcBmp->GetPixelFormat().format)
        {
            return;
        }
        else {
            this->bmpForCopy = nullptr;
        }
    }

    if (!this->bmpForCopy) {
        HRESULT hr = S_OK;

        hr = ctx->CreateBitmap(srcRectSize, D2D1::BitmapProperties(srcBmp->GetPixelFormat()), &this->bmpForCopy);
        H::System::ThrowIfFailed(hr);
    }
}

D2D1_RECT_F D2DBitmapCopy::MakeRect(const D2D1_SIZE_U& size) {
    return D2D1::RectF(0.f, 0.f, static_cast<float>(size.width), static_cast<float>(size.height));
}

D2D1_RECT_F D2DBitmapCopy::Intersect(const D2D1_RECT_F& a, const D2D1_RECT_F& b) {
    auto left = (std::max)(a.left, b.left);
    auto top = (std::max)(a.top, b.top);
    auto right = (std::min)(a.right, b.right);
    auto bottom = (std::min)(a.bottom, b.bottom);

    return D2D1::RectF(left, top, right, bottom);
}

D2D1_SIZE_F D2DBitmapCopy::Size(const D2D1_RECT_F& r) {
    return D2D1::SizeF(r.right - r.left, r.bottom - r.top);
}

D2D1_SIZE_U D2DBitmapCopy::Size(const D2D1_RECT_U& r) {
    return D2D1::SizeU(r.right - r.left, r.bottom - r.top);
}

D2D1_RECT_F D2DBitmapCopy::Round(const D2D1_RECT_F& r) {
    // example:
    // 1, 1, 3, 3 -> 1, 1, 3, 3
    // 0.9, 0.3, 3.9, 3.3 -> 0, 0, 4, 4
    return D2D1::RectF(std::floor(r.left), std::floor(r.top), std::ceil(r.right), std::ceil(r.bottom));
}

#include "pch.h"
#include "MFOutputTexResizeRgbaToNV12.h"

#include <libhelpers/HSystem.h>
#include <libhelpers/HMath.h>

void MFOutputTexResizeRgbaToNV12::Resize(
    ID3D11Device *dev,
    ID3D11DeviceContext *ctx,
    ID3D11Texture2D *dst,
    uint32_t /*dstSubResource*/,
    ID3D11Texture2D* src)
{
    if (this->vproc.vproc) {
        D3D11_TEXTURE2D_DESC dstDesc, srcDesc;

        dst->GetDesc(&dstDesc);
        src->GetDesc(&srcDesc);

        bool inputSame = this->vproc.inputSize.width == srcDesc.Width && this->vproc.inputSize.height == srcDesc.Height;
        bool outputSame = this->vproc.outputSize.width == dstDesc.Width && this->vproc.outputSize.height == dstDesc.Height;

        if (!inputSame || !outputSame) {
            this->vproc = CreateVideoProcessorResult();
        }
    }

    if (!this->vproc.vproc) {
        this->vproc = MFOutputTexResizeRgbaToNV12::CreateVideoProcessor(dev, dst, src);
    }
    auto inputView = CreateInputView(dev, this->vproc.vprocEnum.Get(), src);
    auto outputView = CreateOutputView(dev, this->vproc.vprocEnum.Get(), dst);

    HRESULT hr = S_OK;
    D3D11_TEXTURE2D_DESC texDesc;
    D3D11_TEXTURE2D_DESC dstTexDesc;

    src->GetDesc(&texDesc);
    dst->GetDesc(&dstTexDesc);

    bool sameSize = texDesc.Width == dstTexDesc.Width && texDesc.Height == dstTexDesc.Height;

    Microsoft::WRL::ComPtr<ID3D11VideoContext> vctx;

    hr = ctx->QueryInterface(vctx.GetAddressOf());
    H::System::ThrowIfFailed(hr);

    D3D11_VIDEO_PROCESSOR_STREAM streams = { 0 };
    streams.Enable = TRUE;
    streams.pInputSurface = inputView.Get();

    if (!sameSize)
    {
        RECT srcRect;

        srcRect.left = 0;
        srcRect.top = 0;
        srcRect.right = texDesc.Width;
        srcRect.bottom = texDesc.Height;

        RECT dstRect = MFOutputTexResizeRgbaToNV12::GetDstRect(
            D2D1::SizeU(dstTexDesc.Width, dstTexDesc.Height),
            D2D1::SizeU(texDesc.Width, texDesc.Height));

        vctx->VideoProcessorSetStreamSourceRect(vproc.vproc.Get(), 0, TRUE, &srcRect);
        vctx->VideoProcessorSetStreamDestRect(vproc.vproc.Get(), 0, TRUE, &dstRect);
    }

    hr = vctx->VideoProcessorBlt(vproc.vproc.Get(), outputView.Get(), 0, 1, &streams);
    H::System::ThrowIfFailed(hr);
}

bool MFOutputTexResizeRgbaToNV12::TestNV12Support(
    const D2D1_SIZE_U &dstSize,
    const D2D1_SIZE_U &srcSize)
{
    try {
        auto dxDeviceSafeObjTmp = DxDevice{
            std::make_unique<HH::Mutex<DxDeviceMutex_t>>(),
            std::make_unique<HH::Dx::details::DxDeviceMF>()
        };

        auto vproc = MFOutputTexResizeRgbaToNV12::CreateVideoProcessor(
            dxDeviceSafeObjTmp.Lock()->GetD3DDevice().Get(),
            dstSize, srcSize,
            DXGI_FORMAT_NV12, DXGI_FORMAT_B8G8R8A8_UNORM);

        return true;
    }
    catch (...) {
        return false;
    }
}

MFOutputTexResizeRgbaToNV12::CreateVideoProcessorResult MFOutputTexResizeRgbaToNV12::CreateVideoProcessor(
    ID3D11Device *dev,
    ID3D11Texture2D *dst,
    ID3D11Texture2D *src)
{
    D3D11_TEXTURE2D_DESC dstDesc, srcDesc;

    dst->GetDesc(&dstDesc);
    src->GetDesc(&srcDesc);

    return MFOutputTexResizeRgbaToNV12::CreateVideoProcessor(
        dev,
        D2D1::SizeU(dstDesc.Width, dstDesc.Height),
        D2D1::SizeU(srcDesc.Width, srcDesc.Height),
        dstDesc.Format,
        srcDesc.Format);
}

MFOutputTexResizeRgbaToNV12::CreateVideoProcessorResult MFOutputTexResizeRgbaToNV12::CreateVideoProcessor(
    ID3D11Device *dev,
    const D2D1_SIZE_U &dstSize,
    const D2D1_SIZE_U &srcSize,
    DXGI_FORMAT dstFormat,
    DXGI_FORMAT srcFormat)
{
    D3D11_VIDEO_PROCESSOR_CONTENT_DESC contentDesc;

    contentDesc.Usage = D3D11_VIDEO_USAGE_PLAYBACK_NORMAL;
    contentDesc.InputFrameFormat = D3D11_VIDEO_FRAME_FORMAT_PROGRESSIVE;
    contentDesc.InputFrameRate.Numerator = 1;
    contentDesc.InputFrameRate.Denominator = 1;
    contentDesc.OutputFrameRate = contentDesc.InputFrameRate;
    contentDesc.InputWidth = srcSize.width;
    contentDesc.InputHeight = srcSize.height;
    contentDesc.OutputWidth = dstSize.width;
    contentDesc.OutputHeight = dstSize.height;

    HRESULT hr = S_OK;
    Microsoft::WRL::ComPtr<ID3D11VideoProcessorEnumerator> vprocEnum;
    auto videoDev = MFOutputTexResizeRgbaToNV12::GetVideoDev(dev);

    hr = videoDev->CreateVideoProcessorEnumerator(&contentDesc, vprocEnum.GetAddressOf());
    H::System::ThrowIfFailed(hr);

    uint32_t inputFmtFlags = 0;

    hr = vprocEnum->CheckVideoProcessorFormat(srcFormat, &inputFmtFlags);
    H::System::ThrowIfFailed(hr);

    if (!(inputFmtFlags & D3D11_VIDEO_PROCESSOR_FORMAT_SUPPORT_INPUT)) {
        H::System::ThrowIfFailed(E_FAIL);
    }

    uint32_t outputFmtFlags = 0;

    hr = vprocEnum->CheckVideoProcessorFormat(dstFormat, &outputFmtFlags);
    H::System::ThrowIfFailed(hr);

    if (!(outputFmtFlags & D3D11_VIDEO_PROCESSOR_FORMAT_SUPPORT_OUTPUT)) {
        H::System::ThrowIfFailed(E_FAIL);
    }

    Microsoft::WRL::ComPtr<ID3D11VideoProcessor> vproc;

    hr = videoDev->CreateVideoProcessor(vprocEnum.Get(), 0, vproc.GetAddressOf());
    H::System::ThrowIfFailed(hr);

    CreateVideoProcessorResult res;

    res.inputSize.width = contentDesc.InputWidth;
    res.inputSize.height = contentDesc.InputHeight;
    res.outputSize.width = contentDesc.OutputWidth;
    res.outputSize.height = contentDesc.OutputHeight;
    res.vproc = std::move(vproc);
    res.vprocEnum = std::move(vprocEnum);

    return res;
}

Microsoft::WRL::ComPtr<ID3D11VideoProcessorInputView> MFOutputTexResizeRgbaToNV12::CreateInputView(
    ID3D11Device *dev,
    ID3D11VideoProcessorEnumerator *vprocEnum,
    ID3D11Texture2D *src)
{
    HRESULT hr = S_OK;
    auto videoDev = GetVideoDev(dev);
    Microsoft::WRL::ComPtr<ID3D11VideoProcessorInputView> res;
    D3D11_VIDEO_PROCESSOR_INPUT_VIEW_DESC viewDesc;

    viewDesc.FourCC = 0;
    viewDesc.ViewDimension = D3D11_VPIV_DIMENSION_TEXTURE2D;
    viewDesc.Texture2D.ArraySlice = 0;
    viewDesc.Texture2D.MipSlice = 0;

    hr = videoDev->CreateVideoProcessorInputView(src, vprocEnum, &viewDesc, res.GetAddressOf());
    H::System::ThrowIfFailed(hr);

    return res;
}

Microsoft::WRL::ComPtr<ID3D11VideoProcessorOutputView> MFOutputTexResizeRgbaToNV12::CreateOutputView(
    ID3D11Device *dev,
    ID3D11VideoProcessorEnumerator *vprocEnum,
    ID3D11Texture2D *dst)
{
    HRESULT hr = S_OK;
    auto videoDev = MFOutputTexResizeRgbaToNV12::GetVideoDev(dev);
    Microsoft::WRL::ComPtr<ID3D11VideoProcessorOutputView> res;
    D3D11_VIDEO_PROCESSOR_OUTPUT_VIEW_DESC viewDesc;

    viewDesc.ViewDimension = D3D11_VPOV_DIMENSION_TEXTURE2D;
    viewDesc.Texture2D.MipSlice = 0;

    hr = videoDev->CreateVideoProcessorOutputView(dst, vprocEnum, &viewDesc, res.GetAddressOf());
    H::System::ThrowIfFailed(hr);

    return res;
}

Microsoft::WRL::ComPtr<ID3D11VideoDevice> MFOutputTexResizeRgbaToNV12::GetVideoDev(ID3D11Device *dev) {
    HRESULT hr = S_OK;

    Microsoft::WRL::ComPtr<ID3D11VideoDevice> videoDev;
    hr = dev->QueryInterface(videoDev.GetAddressOf());
    H::System::ThrowIfFailed(hr);

    return videoDev;
}

RECT MFOutputTexResizeRgbaToNV12::GetDstRect(
    const D2D1_SIZE_U &dstSize,
    const D2D1_SIZE_U &srcSize)
{
    auto size = H::Math::InscribeRectAR(
        DirectX::XMFLOAT2((float)dstSize.width, (float)dstSize.height),
        DirectX::XMFLOAT2((float)srcSize.width, (float)srcSize.height));

    D2D1_POINT_2F center;

    center.x = (float)dstSize.width / 2.f;
    center.y = (float)dstSize.height / 2.f;

    D2D1_SIZE_F half;

    half.width = size.x / 2.f;
    half.height = size.y / 2.f;

    D2D1_RECT_F dstRect;

    dstRect.left = center.x - half.width;
    dstRect.top = center.y - half.height;
    dstRect.right = center.x + half.width;
    dstRect.bottom = center.y + half.height;

    RECT resRect;

    resRect.left = (LONG)(std::floor(dstRect.left));
    resRect.top = (LONG)(std::floor(dstRect.top));
    resRect.right = (LONG)(std::ceil(dstRect.right));
    resRect.bottom = (LONG)(std::ceil(dstRect.bottom));

    resRect.left = H::Math::Clamp(resRect.left, 0L, (long)dstSize.width);
    resRect.top = H::Math::Clamp(resRect.top, 0L, (long)dstSize.height);
    resRect.right = H::Math::Clamp(resRect.right, 0L, (long)dstSize.width);
    resRect.bottom = H::Math::Clamp(resRect.bottom, 0L, (long)dstSize.height);

    return resRect;
}

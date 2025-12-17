#pragma once
#include "IMFOutputTexResize.h"

class MFOutputTexResizeRgbaToNV12 : public IMFOutputTexResize {
public:
    void Resize(
        ID3D11Device *dev,
        ID3D11DeviceContext *ctx,
        ID3D11Texture2D *dst,
        uint32_t dstSubResource,
        ID3D11Texture2D* src) override;

    static bool TestNV12Support(
        const D2D1_SIZE_U &dstSize,
        const D2D1_SIZE_U &srcSize);

private:
    struct CreateVideoProcessorResult {
        Microsoft::WRL::ComPtr<ID3D11VideoProcessor> vproc;
        Microsoft::WRL::ComPtr<ID3D11VideoProcessorEnumerator> vprocEnum;
        D2D1_SIZE_U inputSize;
        D2D1_SIZE_U outputSize;
    };

    CreateVideoProcessorResult vproc;

    static CreateVideoProcessorResult CreateVideoProcessor(
        ID3D11Device *dev,
        ID3D11Texture2D *dst,
        ID3D11Texture2D *src);
    static CreateVideoProcessorResult CreateVideoProcessor(
        ID3D11Device *dev,
        const D2D1_SIZE_U &dstSize,
        const D2D1_SIZE_U &srcSize,
        DXGI_FORMAT dstFormat,
        DXGI_FORMAT srcFormat);
    static Microsoft::WRL::ComPtr<ID3D11VideoProcessorInputView> CreateInputView(
        ID3D11Device *dev,
        ID3D11VideoProcessorEnumerator *vprocEnum,
        ID3D11Texture2D *src);
    static Microsoft::WRL::ComPtr<ID3D11VideoProcessorOutputView> CreateOutputView(
        ID3D11Device *dev,
        ID3D11VideoProcessorEnumerator *vprocEnum,
        ID3D11Texture2D *dst);
    static Microsoft::WRL::ComPtr<ID3D11VideoDevice> GetVideoDev(ID3D11Device *dev);

    static RECT GetDstRect(
        const D2D1_SIZE_U &dstSize,
        const D2D1_SIZE_U &srcSize);
};

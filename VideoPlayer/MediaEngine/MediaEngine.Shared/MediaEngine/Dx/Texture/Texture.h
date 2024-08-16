//#pragma once
//#include "TextureDesc.h"
//#include "TextureRTState.h"
//#include "TextureType.h"
//
//#include <Helpers/Dx/DxDevice.h>
////#include <libhelpers\Filesystem.h>
////#include <libhelpers\raw_ptr.h>
////#include <libhelpers\ImageContainerEnum.h>
//#include "BmpSize.h"
//
//#include <vector>
//#include <optional>
//#include <array>
//
//struct TextureCopiedResources {
//    Microsoft::WRL::ComPtr<ID2D1Bitmap1> cpyBmp;
//    D2D1_MAPPED_RECT mappedData;
//
//    TextureCopiedResources() {
//    }
//
//    ~TextureCopiedResources() {
//        this->cpyBmp->Unmap();
//    }
//};
//
//class Texture {
//public:
//    template<class InternalStateT>
//    class TextureRtvStateDeleter {
//    public:
//        TextureRtvStateDeleter() = default;
//        TextureRtvStateDeleter(const Dx::DxDeviceCtxSafeObj_t* dxDevCtxSafeObj, InternalStateT internalState)
//            : dxDevCtxSafeObj(dxDevCtxSafeObj)
//            , internalState(std::move(internalState))
//        {}
//        TextureRtvStateDeleter(ID3D11DeviceContext* d3dCtx, InternalStateT internalState)
//            : d3dCtx(d3dCtx)
//            , internalState(std::move(internalState))
//        {}
//
//        void operator()(Texture* tex) {
//            if (this->dxDevCtxSafeObj) {
//                tex->ResolveMSAA(this->dxDevCtxSafeObj->Lock());
//            }
//            else {
//                tex->ResolveMSAA(this->d3dCtx);
//            }
//        }
//
//        InternalStateT&& DetachInternal() {
//            return std::move(this->internalState);
//        }
//
//    private:
//        const Dx::DxDeviceCtxSafeObj_t* dxDevCtxSafeObj = nullptr;
//        ID3D11DeviceContext* d3dCtx = nullptr;
//        InternalStateT internalState;
//    };
//
//    template<class InternalStateT>
//    using TextureRtvState = std::unique_ptr<Texture, TextureRtvStateDeleter<InternalStateT>>;
//
//    Texture(TextureType type, bool isRtv = false, bool bitmap2D = false, bool useMSAA = false);
//
//    bool Empty() const;
//    TextureType GetType() const;
//    BmpSize GetSize() const;
//
//    void Resize(Dx::DxDeviceSafeObj::_Locked& dxDev, const BmpSize& size, uint32_t mipCount = 1, const void** data = nullptr);
//    void ResolveMSAA(Dx::DxDeviceCtxSafeObj_t::_Locked& dxDevCtx, bool force = false);
//    void ResolveMSAA(ID3D11DeviceContext* d3dCtx, bool force = false);
//    void UpdateData(Dx::DxDeviceCtxSafeObj_t::_Locked& dxDevCtx, const void* data, size_t size);
//    void UpdateMipmap(Dx::DxDeviceCtxSafeObj_t::_Locked& dxDevCtx);
//
//    void SetD3DTexture(Dx::DxDeviceSafeObj::_Locked& dxDev, Microsoft::WRL::ComPtr<ID3D11Texture2D> tex, float dpiX = BmpSize::DefaultDpi.x, float dpiY = BmpSize::DefaultDpi.y);
//
//    void SetToContext(ID3D11DeviceContext* d3dCtx, uint32_t startSlot = 0);
//    void SetToContext(Dx::DxDeviceCtxSafeObj_t::_Locked& dxDevCtx, uint32_t startSlot = 0);
//    TextureRtvState<D3DTargetState<1>> SetToContextRtvScoped(ID3D11DeviceContext* d3dCtx);
//    TextureRtvState<D2DTargetState> SetToContextRtvScopedD2D(Dx::DxDeviceCtxSafeObj_t::_Locked& dxDevCtx);
//    // sets both d3d and d2d render targets
//    TextureRtvState<TextureRTState> SetToContextRtvScoped(Dx::DxDeviceCtxSafeObj_t::_Locked& dxDevCtx);
//    void Clear(ID3D11DeviceContext* d3dCtx, const float color[4]);
//    void GetResource(ID3D11Resource** resource);
//
//    //void GetBitmapBrush(D2DCtxMt* d2dCtxMt, ID2D1BitmapBrush1** brush);
//    Microsoft::WRL::ComPtr<ID2D1Bitmap1> GetBitmap(size_t planeIdx = 0);
//
//    TextureCopiedResources GetData(ID2D1DeviceContext* d2dCtx, size_t planeIdx = 0);
//    static TextureCopiedResources GetData(ID2D1DeviceContext* d2dCtx, const Microsoft::WRL::ComPtr<ID2D1Bitmap1>& bmp);
//    std::vector<uint8_t> GetDataAsVector(ID2D1DeviceContext* d2dCtx, size_t planeIdx = 0);
//    //void SaveToFile(ID2D1DeviceContext* d2dCtx, raw_ptr<Filesystem::IStream> stream, ImageContainer format = ImageContainer::PNG);
//    //static void SaveToFile(ID2D1DeviceContext* d2dCtx, raw_ptr<Filesystem::IStream> stream, const Microsoft::WRL::ComPtr<ID2D1Bitmap1>& bmp, const DirectX::XMUINT2& size, ImageContainer format = ImageContainer::PNG);
//
//    //void LoadFromFile(ID3D11Device* d3dDev, D2DCtxMt* d2dCtxMt, Filesystem::IStream* stream);
//
//private:
//    std::optional<UINT> GetMsaaLevelForTexture(ID3D11Device* d3dDev) const;
//    void AssertIsMsaaResolved() const;
//
//    uint32_t mipCount = 1;
//    TextureType type = TextureType::Unknown;
//    BmpSize size = {};
//    size_t byteSize = 0;
//    bool isRtv = false;
//    bool bitmap2D = false;
//    // http://stackoverflow.com/questions/20674415/extra-bytes-on-the-end-of-yuv-buffer-raspberrypi
//    // TODO check if padded textures have bad pixels at right and bottom side
//    DirectX::XMUINT2 sizePadding = {}; // in pixels
//    size_t byteSizePadding = 0;
//    bool useMSAA = false; // if Texture should try to use MSAA
//    bool msaaAvalable = false; // if Texture actually uses MSAA
//    bool msaaResolved = false; // if planeRtvMSAA resolved to non MSAA planeRtv after recent call to SetToContextRtvScoped/SetToContextRtvScopedD2D
//    std::vector<Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>> planeSrv;
//    std::vector<Microsoft::WRL::ComPtr<ID3D11RenderTargetView>> planeRtv;
//    std::vector<Microsoft::WRL::ComPtr<ID2D1Bitmap1>> bimaps;
//
//    std::vector<Microsoft::WRL::ComPtr<ID3D11RenderTargetView>> planeRtvMSAA;
//    std::vector<Microsoft::WRL::ComPtr<ID2D1Bitmap1>> bimapsMSAA;
//};

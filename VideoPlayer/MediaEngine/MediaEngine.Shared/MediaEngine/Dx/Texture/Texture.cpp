//#include "Texture.h"
//#include "../../HMath.h"
//#include "../DxHelpers.h"
//#include <Helpers/System.h>
////#include <libhelpers\ImageUtils.h>
//
//Texture::Texture(TextureType type, bool isRtv /*= false*/, bool bitmap2D /*= false*/, bool useMSAA /*= false*/)
//    : type(type), isRtv(isRtv), bitmap2D(bitmap2D), useMSAA(useMSAA)
//{}
//
//bool Texture::Empty() const {
//    bool empty =
//        this->size.GetLogicalWidth() == 0 ||
//        this->size.GetLogicalHeight() == 0;
//
//    return empty;
//}
//
//TextureType Texture::GetType() const {
//    return this->type;
//}
//
//BmpSize Texture::GetSize() const {
//    return this->size;
//}
//
//void Texture::Resize(Dx::DxDeviceSafeObj::_Locked& dxDev, const BmpSize& size, uint32_t mipCount, const void** data) {
//    auto d3dDevice = dxDev->GetD3DDevice();
//    auto dxCtx = dxDev->LockContext();
//    auto d2dCtx = dxCtx->D2D();
//    auto d3dCtx = dxCtx->D3D();
//
//    if (this->size != size || this->mipCount != mipCount || this->planeSrv.empty()) {
//        this->size = size;
//        this->mipCount = mipCount;
//
//        this->byteSize = 0;
//        this->planeSrv.clear();
//        this->planeRtv.clear();
//        this->bimaps.clear();
//        this->planeRtvMSAA.clear();
//        this->bimapsMSAA.clear();
//        this->msaaResolved = false;
//
//        if (!this->Empty()) {
//            auto planeCount = TextureDesc::GetPlaneCount(this->type);
//            HRESULT hr = S_OK;
//            UINT msaaLevel = 0;
//
//            if (auto supportedMsaaLevel = this->GetMsaaLevelForTexture(d3dDevice.Get())) {
//                msaaLevel = *supportedMsaaLevel;
//                this->msaaAvalable = true;
//            }
//            else {
//                this->msaaAvalable = false;
//            }
//
//            for (size_t p = 0; p < planeCount; p++) {
//                auto planeInfo = TextureDesc::GetPlaneInfo(this->type, p);
//                Microsoft::WRL::ComPtr<ID3D11Texture2D> tex;
//                Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv;
//                D3D11_TEXTURE2D_DESC texDesc;
//
//                texDesc.Width = this->size.GetPhysicalWidthUint() / planeInfo.WidthSamplePeriod;
//                texDesc.Height = this->size.GetPhysicalHeightUint() / planeInfo.HeightSamplePeriod;
//                texDesc.MipLevels = this->mipCount;
//                texDesc.ArraySize = 1;
//                texDesc.Format = TextureDesc::GetDxFormat(this->type, p);
//                texDesc.Usage = D3D11_USAGE_DEFAULT;
//                texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | (this->isRtv || this->mipCount > 1 ? D3D11_BIND_RENDER_TARGET : 0);
//                texDesc.CPUAccessFlags = 0;
//                texDesc.MiscFlags = this->mipCount > 1 ? D3D11_RESOURCE_MISC_GENERATE_MIPS : 0;
//                texDesc.SampleDesc.Count = 1;
//                texDesc.SampleDesc.Quality = 0;
//
//                D3D11_SUBRESOURCE_DATA subResData, * subResDataPtr = nullptr;
//                if (data) {
//                    subResData.pSysMem = data[p];
//                    subResData.SysMemPitch = (texDesc.Width * planeInfo.Component.BitSize * planeInfo.Component.Count) / 8;
//                    subResDataPtr = &subResData;
//                }
//
//                hr = d3dDevice->CreateTexture2D(&texDesc, subResDataPtr, tex.GetAddressOf());
//                H::System::ThrowIfFailed(hr);
//
//                hr = d3dDevice->CreateShaderResourceView(tex.Get(), nullptr, srv.GetAddressOf());
//                H::System::ThrowIfFailed(hr);
//
//                this->planeSrv.push_back(srv);
//
//                if (this->isRtv) {
//                    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> rtv;
//
//                    hr = d3dDevice->CreateRenderTargetView(tex.Get(), nullptr, rtv.GetAddressOf());
//                    H::System::ThrowIfFailed(hr);
//
//                    this->planeRtv.push_back(rtv);
//                }
//
//                Microsoft::WRL::ComPtr<ID3D11Texture2D> texMSAA;
//
//                if (this->msaaAvalable) {
//                    D3D11_TEXTURE2D_DESC texDescMSAA = texDesc;
//
//                    auto sampleDescMSAA = DxHelpers::MsaaHelper::GetMaxSupportedMSAA(d3dDevice.Get(), texDescMSAA.Format, msaaLevel);
//                    if (!sampleDescMSAA) {
//                        // if failed need to check GetMsaaLevelForTexture logic
//                        assert(false);
//                        H::System::ThrowIfFailed(E_FAIL);
//                    }
//
//                    texDescMSAA.SampleDesc = *sampleDescMSAA;
//
//                    hr = d3dDevice->CreateTexture2D(&texDescMSAA, nullptr, texMSAA.GetAddressOf());
//                    if (FAILED(hr)) {
//                        // if failed need to check GetMsaaLevelForTexture logic
//                        assert(false);
//                    }
//                    H::System::ThrowIfFailed(hr);
//
//                    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> rtvMSAA;
//
//                    hr = d3dDevice->CreateRenderTargetView(texMSAA.Get(), nullptr, rtvMSAA.GetAddressOf());
//                    if (FAILED(hr)) {
//                        // if failed need to check GetMsaaLevelForTexture logic
//                        assert(false);
//                    }
//                    H::System::ThrowIfFailed(hr);
//
//                    this->planeRtvMSAA.push_back(rtvMSAA);
//                }
//
//                if (this->bitmap2D) {
//                    Microsoft::WRL::ComPtr<IDXGISurface2> surface;
//                    Microsoft::WRL::ComPtr<ID2D1Bitmap1> bitmap;
//
//                    hr = tex.As(&surface);
//                    H::System::ThrowIfFailed(hr);
//
//                    D2D1_BITMAP_OPTIONS bmpOpts = this->isRtv ? D2D1_BITMAP_OPTIONS_TARGET : D2D1_BITMAP_OPTIONS_NONE;
//
//                    D2D1_BITMAP_PROPERTIES1 bitmapProperties =
//                        D2D1::BitmapProperties1(
//                            bmpOpts,
//                            D2D1::PixelFormat(texDesc.Format, D2D1_ALPHA_MODE_PREMULTIPLIED),
//                            this->size.GetLogicalDpiX(),
//                            this->size.GetLogicalDpiY()
//                        );
//
//                    hr = d2dCtx->CreateBitmapFromDxgiSurface(surface.Get(), &bitmapProperties, bitmap.GetAddressOf());
//                    H::System::ThrowIfFailed(hr);
//
//                    this->bimaps.push_back(bitmap);
//
//                    if (this->msaaAvalable) {
//                        Microsoft::WRL::ComPtr<IDXGISurface2> surfaceMSAA;
//                        Microsoft::WRL::ComPtr<ID2D1Bitmap1> bitmapMSAA;
//
//                        hr = texMSAA.As(&surfaceMSAA);
//                        H::System::ThrowIfFailed(hr);
//
//                        hr = d2dCtx->CreateBitmapFromDxgiSurface(surfaceMSAA.Get(), &bitmapProperties, bitmapMSAA.GetAddressOf());
//                        H::System::ThrowIfFailed(hr);
//
//                        this->bimapsMSAA.push_back(bitmapMSAA);
//                    }
//                }
//
//                auto& conponentInfo = planeInfo.Component;
//                size_t planeBitSize = texDesc.Width * texDesc.Height * conponentInfo.Count * conponentInfo.BitSize;
//                this->byteSize += planeBitSize / 8;
//            }
//        }
//    }
//}
//
//void Texture::ResolveMSAA(Dx::DxDeviceCtxSafeObj_t::_Locked& dxDevCtx, bool force /*= false*/) {
//    auto d2dCtx = dxDevCtx->D2D();
//    d2dCtx->Flush();
//    ResolveMSAA(dxDevCtx->D3D(), force);
//}
//
//void Texture::ResolveMSAA(ID3D11DeviceContext* d3dCtx, bool force /*= false*/) {
//    if (!this->msaaAvalable) {
//        return;
//    }
//    if (this->msaaResolved && !force) {
//        return;
//    }
//
//    for (size_t i = 0; i < this->planeSrv.size(); ++i) {
//        const auto& resolved = this->planeSrv[i];
//        const auto& msaa = this->planeRtvMSAA[i];
//
//        Microsoft::WRL::ComPtr<ID3D11Resource> msaaRes;
//        Microsoft::WRL::ComPtr<ID3D11Resource> resolvedRes;
//
//        msaa->GetResource(&msaaRes);
//        resolved->GetResource(&resolvedRes);
//
//        HRESULT hr = S_OK;
//        Microsoft::WRL::ComPtr<ID3D11Texture2D> msaaTex;
//
//        hr = msaaRes.As(&msaaTex);
//        H::System::ThrowIfFailed(hr);
//
//        D3D11_TEXTURE2D_DESC texDesc = {};
//
//        msaaTex->GetDesc(&texDesc);
//
//        d3dCtx->ResolveSubresource(resolvedRes.Get(), 0, msaaRes.Get(), 0, texDesc.Format);
//    }
//
//    this->msaaResolved = true;
//}
//
//void Texture::UpdateData(Dx::DxDeviceCtxSafeObj_t::_Locked& dxDevCtx, const void* data, size_t size) {
//    DirectX::XMUINT2 planeSize;
//    auto physSize = this->size.GetPhysicalSizeUint();
//
//    this->msaaResolved = false;
//
//    if (this->byteSize + this->byteSizePadding != size) {
//        // check is padding needed
//        // now only supporting vertical padding
//        // if horizontal padding(for each row) will be detected then texture will be not updated
//
//        // check this only if buffer size is not as expected because only is this case buffer may have some padding.
//
//        // http://stackoverflow.com/questions/20674415/extra-bytes-on-the-end-of-yuv-buffer-raspberrypi
//        /*
//        Note that the image buffers saved in raspistillyuv are padded to a horizontal size divisible by 16
//        (so there may be unused bytes at the end of each line to made the width divisible by 16).
//        Buffers are also padded vertically to be divisible by 16, and in the YUV mode, each plane of Y,U,V is padded in this way.
//
//        So my interpretation of this is the following.
//        The width is 2592 (divisible by 16 so this is ok).
//        The height is 1944 which is 8 short of being divisible by 16 so an extra 8*2592 are added
//        (also multiplied by 1.5) thus giving your 31104 extra bytes.
//        */
//
//        this->sizePadding.x = HMath::RoundUpPOT(physSize.x, 16u) - physSize.x;
//        this->sizePadding.y = HMath::RoundUpPOT(physSize.y, 16u) - physSize.y;
//
//        this->byteSizePadding = 0;
//
//        auto planeCount = TextureDesc::GetPlaneCount(this->type);
//
//        for (size_t i = 0; i < planeCount; i++) {
//            auto planeInfo = TextureDesc::GetPlaneInfo(this->type, i);
//            auto& conponentInfo = planeInfo.Component;
//
//            planeSize.x = physSize.x / planeInfo.WidthSamplePeriod;
//            planeSize.y = physSize.y / planeInfo.HeightSamplePeriod;
//
//            DirectX::XMUINT2 planePadding;
//
//            planePadding.x = this->sizePadding.x / planeInfo.WidthSamplePeriod;
//            planePadding.y = this->sizePadding.y / planeInfo.HeightSamplePeriod;
//
//            auto totalPaddedPixels = (planePadding.x * planeSize.y) + ((planeSize.x + planePadding.x) * planePadding.y);
//
//            this->byteSizePadding += (totalPaddedPixels * conponentInfo.Count * conponentInfo.BitSize) / 8;
//        }
//    }
//
//    if (this->byteSize + this->byteSizePadding == size) {
//        size_t bytesPos = 0;
//        const uint8_t* bytes = static_cast<const uint8_t*>(data);
//        auto d3dCtx = dxDevCtx->D3D();
//        auto planeCount = TextureDesc::GetPlaneCount(this->type);
//
//        for (size_t i = 0; i < this->planeSrv.size(); i++) {
//            Microsoft::WRL::ComPtr<ID3D11Resource> res;
//            auto planeInfo = TextureDesc::GetPlaneInfo(this->type, i);
//            auto& conponentInfo = planeInfo.Component;
//
//            this->planeSrv[i]->GetResource(res.GetAddressOf());
//
//            planeSize.x = (physSize.x + this->sizePadding.x) / planeInfo.WidthSamplePeriod;
//            planeSize.y = (physSize.y + this->sizePadding.y) / planeInfo.HeightSamplePeriod;
//
//            size_t rowPitch = (planeSize.x * conponentInfo.Count * conponentInfo.BitSize) / 8;
//            size_t dataSize = rowPitch * planeSize.y;
//
//            d3dCtx->UpdateSubresource(
//                res.Get(),
//                0,
//                nullptr,
//                &bytes[bytesPos],
//                static_cast<uint32_t>(rowPitch),
//                static_cast<uint32_t>(dataSize));
//
//            bytesPos += dataSize;
//
//            if (this->mipCount > 1) {
//                d3dCtx->GenerateMips(this->planeSrv[i].Get());
//            }
//        }
//    }
//}
//
//void Texture::UpdateMipmap(Dx::DxDeviceCtxSafeObj_t::_Locked& dxDevCtx) {
//    if (this->mipCount > 1) {
//        auto d3dCtx = dxDevCtx->D3D();
//
//        for (size_t i = 0; i < this->planeSrv.size(); i++) {
//            d3dCtx->GenerateMips(this->planeSrv[i].Get());
//        }
//    }
//}
//
//void Texture::SetD3DTexture(Dx::DxDeviceSafeObj::_Locked& dxDev, Microsoft::WRL::ComPtr<ID3D11Texture2D> tex, float dpiX, float dpiY) {
//    HRESULT hr = S_OK;
//
//    auto d3dDevice = dxDev->GetD3DDevice();
//    auto dxCtx = dxDev->LockContext();
//    auto d2dCtx = dxCtx->D2D();
//    auto d3dCtx = dxCtx->D3D();
//
//    this->byteSize = 0;
//    this->planeSrv.clear();
//    this->planeRtv.clear();
//    this->bimaps.clear();
//    this->planeRtvMSAA.clear();
//    this->bimapsMSAA.clear();
//    this->msaaResolved = false;
//    this->msaaAvalable = false; // MSAA available only when plane textures created by Texture class
//
//    D3D11_TEXTURE2D_DESC texDesc;
//    tex->GetDesc(&texDesc);
//
//    D3D11_SHADER_RESOURCE_VIEW_DESC shaderdResourceViewDesc;
//    shaderdResourceViewDesc.Format = texDesc.Format;
//    shaderdResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
//    shaderdResourceViewDesc.Texture2D.MipLevels = texDesc.MipLevels;
//    shaderdResourceViewDesc.Texture2D.MostDetailedMip = 0;
//
//    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> shaderdResourceView;
//    hr = d3dDevice->CreateShaderResourceView(tex.Get(), &shaderdResourceViewDesc, shaderdResourceView.GetAddressOf());
//    H::System::ThrowIfFailed(hr);
//
//    this->planeSrv.push_back(shaderdResourceView);
//
//
//    if (this->isRtv) {
//        D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
//        rtvDesc.Format = texDesc.Format;
//        rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
//        rtvDesc.Texture2D.MipSlice = 0;
//
//        Microsoft::WRL::ComPtr<ID3D11RenderTargetView> rtv;
//        hr = d3dDevice->CreateRenderTargetView(tex.Get(), &rtvDesc, rtv.GetAddressOf());
//        H::System::ThrowIfFailed(hr);
//
//        this->planeRtv.push_back(rtv);
//    }
//
//    if (this->bitmap2D) {
//        Microsoft::WRL::ComPtr<IDXGISurface2> surface;
//        Microsoft::WRL::ComPtr<ID2D1Bitmap1> bitmap;
//
//        hr = tex.As(&surface);
//
//        D2D1_BITMAP_OPTIONS bmpOpts = this->isRtv ? D2D1_BITMAP_OPTIONS_TARGET : D2D1_BITMAP_OPTIONS_NONE;
//
//        D2D1_BITMAP_PROPERTIES1 bitmapProperties =
//            D2D1::BitmapProperties1(
//                bmpOpts,
//                D2D1::PixelFormat(texDesc.Format, D2D1_ALPHA_MODE_PREMULTIPLIED),
//                dpiX,
//                dpiY
//            );
//
//        hr = d2dCtx->CreateBitmapFromDxgiSurface(surface.Get(), &bitmapProperties, bitmap.GetAddressOf());
//        H::System::ThrowIfFailed(hr);
//
//        this->bimaps.push_back(bitmap);
//    }
//
//    auto planeInfo = TextureDesc::GetPlaneInfo(this->type, 0);
//    auto& conponentInfo = planeInfo.Component;
//    size_t planeBitSize = texDesc.Width * texDesc.Height * conponentInfo.Count * conponentInfo.BitSize;
//    this->byteSize += planeBitSize / 8;
//
//    this->size.SetPhysicalWidth((float)texDesc.Width, dpiX);
//    this->size.SetPhysicalHeight((float)texDesc.Height, dpiY);
//}
//
//void Texture::SetToContext(ID3D11DeviceContext* d3dCtx, uint32_t startSlot) {
//    this->ResolveMSAA(d3dCtx);
//
//    for (size_t i = 0; i < this->planeSrv.size(); i++) {
//        d3dCtx->PSSetShaderResources(startSlot + (uint32_t)i, 1, this->planeSrv[i].GetAddressOf());
//    }
//}
//
//void Texture::SetToContext(Dx::DxDeviceCtxSafeObj_t::_Locked& dxDevCtx, uint32_t startSlot) {
//    auto d2dCtx = dxDevCtx->D2D();
//    d2dCtx->Flush();
//    this->SetToContext(dxDevCtx->D3D(), startSlot);
//}
//
//Texture::TextureRtvState<D3DTargetState<1>> Texture::SetToContextRtvScoped(ID3D11DeviceContext* d3dCtx) {
//    D3DTargetState<1> state(d3dCtx);
//
//    this->msaaResolved = false;
//    auto& currentPlaneRtv = this->msaaAvalable ? this->planeRtvMSAA : this->planeRtv;
//
//    if (!currentPlaneRtv.empty()) {
//        D3D11_VIEWPORT viewport;
//
//        viewport.TopLeftX = viewport.TopLeftY = 0;
//        viewport.Width = this->size.GetPhysicalWidth();
//        viewport.Height = this->size.GetPhysicalHeight();
//        viewport.MinDepth = 0.0f;
//        viewport.MaxDepth = 1.0f;
//
//        d3dCtx->OMSetRenderTargets(1, currentPlaneRtv[0].GetAddressOf(), nullptr);
//        d3dCtx->RSSetViewports(1, &viewport);
//    }
//
//    return TextureRtvState<D3DTargetState<1>>(this, TextureRtvStateDeleter<D3DTargetState<1>>(d3dCtx, std::move(state)));
//}
//
//Texture::TextureRtvState<D2DTargetState> Texture::SetToContextRtvScopedD2D(Dx::DxDeviceCtxSafeObj_t::_Locked& dxDevCtx) {
//    auto d2dCtx = dxDevCtx->D2D();
//    D2DTargetState state(d2dCtx);
//
//    this->msaaResolved = false;
//    auto& currentBitmaps = this->msaaAvalable ? this->bimapsMSAA : this->bimaps;
//
//    d2dCtx->SetTarget(currentBitmaps[0].Get());
//    d2dCtx->SetDpi(this->size.GetLogicalDpiX(), this->size.GetLogicalDpiY());
//
//    return Texture::TextureRtvState<D2DTargetState>(this, TextureRtvStateDeleter<D2DTargetState>(dxDevCtx.GetCreator(), std::move(state)));
//}
//
//Texture::TextureRtvState<TextureRTState> Texture::SetToContextRtvScoped(Dx::DxDeviceCtxSafeObj_t::_Locked& dxDevCtx) {
//    auto d2dState = this->SetToContextRtvScopedD2D(dxDevCtx);
//    auto d3dState = this->SetToContextRtvScoped(dxDevCtx->D3D());
//
//    TextureRTState state(
//        d2dState.get_deleter().DetachInternal(),
//        d3dState.get_deleter().DetachInternal());
//
//    d2dState.release();
//    d3dState.release();
//
//    return TextureRtvState<TextureRTState>(this, TextureRtvStateDeleter<TextureRTState>(dxDevCtx.GetCreator(), std::move(state)));
//}
//
//void Texture::Clear(ID3D11DeviceContext* d3dCtx, const float color[4]) {
//    this->msaaResolved = false;
//    auto& currentPlaneRtv = this->msaaAvalable ? this->planeRtvMSAA : this->planeRtv;
//
//    for (size_t i = 0; i < currentPlaneRtv.size(); i++) {
//        d3dCtx->ClearRenderTargetView(currentPlaneRtv[i].Get(), color);
//    }
//}
//
//void Texture::GetResource(ID3D11Resource** resource) {
//    this->AssertIsMsaaResolved();
//
//    this->planeSrv[0]->GetResource(resource);
//}
//
////void Texture::GetBitmapBrush(D2DCtxMt* d2dCtxMt, ID2D1BitmapBrush1** brush) {
////    this->AssertIsMsaaResolved();
////
////    auto bitmap = this->bimaps[0];
////    HRESULT hr = d2dCtxMt->CreateBitmapBrush(bitmap.Get(), brush);
////    H::System::ThrowIfFailed(hr);
////}
//
//Microsoft::WRL::ComPtr<ID2D1Bitmap1> Texture::GetBitmap(size_t planeIdx) {
//    this->AssertIsMsaaResolved();
//
//    return this->bimaps[planeIdx];
//}
//
//TextureCopiedResources Texture::GetData(ID2D1DeviceContext* d2dCtx, size_t planeIdx) {
//    this->AssertIsMsaaResolved();
//
//    Microsoft::WRL::ComPtr<ID2D1Bitmap1> bmp = this->GetBitmap(planeIdx);
//    return Texture::GetData(d2dCtx, bmp);
//}
//
//TextureCopiedResources Texture::GetData(ID2D1DeviceContext* d2dCtx, const Microsoft::WRL::ComPtr<ID2D1Bitmap1>& bmp) {
//    TextureCopiedResources res;
//
//    D2D1_BITMAP_PROPERTIES1 prop;
//    bmp->GetDpi(&prop.dpiX, &prop.dpiY);
//    prop.pixelFormat = bmp->GetPixelFormat();
//    prop.bitmapOptions = D2D1_BITMAP_OPTIONS_CPU_READ | D2D1_BITMAP_OPTIONS_CANNOT_DRAW;
//    Microsoft::WRL::ComPtr<ID2D1ColorContext> cc;
//    bmp->GetColorContext(cc.GetAddressOf());
//    prop.colorContext = cc.Get();
//
//    auto textureSize = bmp->GetSize();
//    H::System::ThrowIfFailed(
//        d2dCtx->CreateBitmap(D2D1::SizeU((uint32_t)textureSize.width, (uint32_t)textureSize.height), nullptr, 0, prop, res.cpyBmp.GetAddressOf()));
//    H::System::ThrowIfFailed(
//        res.cpyBmp->CopyFromBitmap(nullptr, bmp.Get(), nullptr));
//
//    H::System::ThrowIfFailed(
//        res.cpyBmp->Map(D2D1_MAP_OPTIONS::D2D1_MAP_OPTIONS_READ, &res.mappedData));
//    return res;
//}
//
//std::vector<uint8_t> Texture::GetDataAsVector(ID2D1DeviceContext* d2dCtx, size_t planeIdx) {
//    this->AssertIsMsaaResolved();
//
//    Microsoft::WRL::ComPtr<ID2D1Bitmap1> bmp = this->GetBitmap(planeIdx);
//    Microsoft::WRL::ComPtr<ID2D1Bitmap1> cpyBmp;
//    std::vector<uint8_t> res;
//
//    D2D1_BITMAP_PROPERTIES1 prop;
//    bmp->GetDpi(&prop.dpiX, &prop.dpiY);
//    prop.pixelFormat = bmp->GetPixelFormat();
//    prop.bitmapOptions = D2D1_BITMAP_OPTIONS_CPU_READ | D2D1_BITMAP_OPTIONS_CANNOT_DRAW;
//    Microsoft::WRL::ComPtr<ID2D1ColorContext> cc;
//    bmp->GetColorContext(cc.GetAddressOf());
//    prop.colorContext = cc.Get();
//
//    auto textureSize = bmp->GetSize();
//    auto texSizeU = D2D1::SizeU((uint32_t)textureSize.width, (uint32_t)textureSize.height);
//    H::System::ThrowIfFailed(
//        d2dCtx->CreateBitmap(texSizeU, nullptr, 0, prop, cpyBmp.GetAddressOf()));
//    H::System::ThrowIfFailed(
//        cpyBmp->CopyFromBitmap(nullptr, bmp.Get(), nullptr));
//
//    D2D1_MAPPED_RECT mappedData;
//
//    res.resize(texSizeU.width * texSizeU.height * 4);
//
//    H::System::ThrowIfFailed(
//        cpyBmp->Map(D2D1_MAP_OPTIONS::D2D1_MAP_OPTIONS_READ, &mappedData));
//
//    if (mappedData.pitch == texSizeU.width * 4) {
//        std::memcpy(res.data(), mappedData.bits, res.size());
//    }
//    else {
//        const uint8_t* src = (const uint8_t*)mappedData.bits;
//        uint8_t* dst = res.data();
//
//        for (uint32_t y = 0; y < texSizeU.height; y++, src += mappedData.pitch, dst += texSizeU.width * 4) {
//            std::memcpy(dst, src, texSizeU.width * 4);
//        }
//    }
//
//    H::System::ThrowIfFailed(
//        cpyBmp->Unmap());
//
//    return res;
//}
//
////void Texture::SaveToFile(ID2D1DeviceContext* d2dCtx, raw_ptr<Filesystem::IStream> stream, ImageContainer format) {
////    this->AssertIsMsaaResolved();
////
////    Microsoft::WRL::ComPtr<ID2D1Bitmap1> bmp = this->GetBitmap(0);
////    DirectX::XMUINT2 sz;
////    auto tmpSz = this->size.GetPhysicalSizeUint();
////
////    sz.x = tmpSz.x;
////    sz.y = tmpSz.y;
////
////    return Texture::SaveToFile(d2dCtx, stream, bmp, sz, format);
////}
////
////void Texture::SaveToFile(ID2D1DeviceContext* d2dCtx, raw_ptr<Filesystem::IStream> stream, const Microsoft::WRL::ComPtr<ID2D1Bitmap1>& bmp, const DirectX::XMUINT2& size, ImageContainer format) {
////    ImageUtils imgUtils;
////    WICPixelFormatGUID dstFmt;
////    Microsoft::WRL::ComPtr<IStream> istream;
////    Microsoft::WRL::ComPtr<IWICBitmap> bitmap;
////    auto data = Texture::GetData(d2dCtx, bmp);
////    auto winStreamExt = dynamic_cast<Filesystem::IWinStreamExt*>(stream.get());
////    winStreamExt->CreateIStream(istream.GetAddressOf());
////    auto encoder = imgUtils.CreateEncoder(istream, format);
////    auto frame = imgUtils.CreateFrameForEncode(encoder.Get());
////
////    imgUtils.EncodeAllocPixels(frame.Get(), size, GUID_WICPixelFormat32bppBGRA, dstFmt);
////
////    if (!IsEqualGUID(dstFmt, GUID_WICPixelFormat32bppBGRA)) {
////        imgUtils.CreateBitmapFromMemory(size.x, size.y, GUID_WICPixelFormat32bppBGRA, data.mappedData.pitch, data.mappedData.pitch * size.y, data.mappedData.bits, bitmap.GetAddressOf());
////
////        auto fmtConv = imgUtils.ConvertPixelFormatInMemory(bitmap.Get(), dstFmt);
////        imgUtils.EncodeFrame(frame.Get(), fmtConv.Get());
////    }
////    else {
////        imgUtils.EncodePixels(frame.Get(), size.y, data.mappedData.pitch, data.mappedData.pitch * size.y, data.mappedData.bits);
////    }
////
////    imgUtils.EncodeCommit(frame.Get());
////    imgUtils.EncodeCommit(encoder.Get());
////}
////
////void Texture::LoadFromFile(ID3D11Device* d3dDev, D2DCtxMt* d2dCtxMt, Filesystem::IStream* stream) {
////    auto pixels = ImageUtils::LoadDxPBgra32Image(stream);
////    BmpSize size(Structs::Float2(static_cast<float>(pixels.Size.x), static_cast<float>(pixels.Size.y)));
////
////    auto imgData = (const void*)pixels.Data.data();
////    this->Resize(d3dDev, d2dCtxMt, size, 1, &imgData);
////}
//
//std::optional<UINT> Texture::GetMsaaLevelForTexture(ID3D11Device* d3dDev) const {
//    UINT msaaLevel = DxHelpers::MsaaHelper::GetMaxMSAA();
//    const auto planeCount = TextureDesc::GetPlaneCount(this->type);
//
//    if (this->useMSAA && this->isRtv) {
//        // Find max MSAA level supported by all texture planes(subtextures)
//        // If one plane doesn't support MSAA then MSAA not available for texture
//        for (size_t p = 0; p < planeCount; p++) {
//            auto format = TextureDesc::GetDxFormat(this->type, p);
//
//            auto planeMsaaLevel = DxHelpers::MsaaHelper::GetMaxSupportedMSAA(d3dDev, format, msaaLevel);
//            if (!planeMsaaLevel) {
//                return {}; // MSAA not available
//            }
//
//            msaaLevel = (std::min)(msaaLevel, planeMsaaLevel->Count);
//        }
//    }
//    else {
//        // MSAA used only for render targets
//        return {}; // MSAA not available
//    }
//
//    return msaaLevel;
//}
//
//void Texture::AssertIsMsaaResolved() const {
//    if (this->msaaAvalable) {
//        // must be resolved to use up to date data
//        // maybe SetToContextRtvScoped state not destroyed by this time
//        assert(this->msaaResolved);
//    }
//}

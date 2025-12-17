#include "pch.h"
//#include "DxDevice.h"
//#include "libhelpers\HSystem.h"
//#include "libhelpers\Macros.h"
//
//#include <dxgi1_3.h>
//
//MFDXGIDeviceManagerCustom::MFDXGIDeviceManagerCustom(Microsoft::WRL::ComPtr<IMFDXGIDeviceManager> mfDxgiDeviceManagerOrig)
//    : mfDxgiDeviceManagerOrig{ mfDxgiDeviceManagerOrig }
//{}
//
//HRESULT __stdcall MFDXGIDeviceManagerCustom::CloseDeviceHandle(HANDLE hDevice) {
//    return mfDxgiDeviceManagerOrig->CloseDeviceHandle(hDevice);
//}
//
//HRESULT __stdcall MFDXGIDeviceManagerCustom::GetVideoService(HANDLE hDevice, REFIID riid, void** ppService) {
//    return mfDxgiDeviceManagerOrig->GetVideoService(hDevice, riid, ppService);
//}
//
//HRESULT __stdcall MFDXGIDeviceManagerCustom::LockDevice(HANDLE hDevice, REFIID riid, void** ppUnkDevice, BOOL fBlock) {
//    ////LOG_DEBUG_D(L"LockDevice [th = ({}) | \"{}\"]", HH::GetThreadId(), HH::ThreadNameHelper::GetThreadName());
//    HRESULT hr = S_OK;
//    hr = mfDxgiDeviceManagerOrig->LockDevice(hDevice, riid, ppUnkDevice, fBlock);
//    //if (SUCCEEDED(hr)) {
//    //    this->d3dMultithread.Reset();
//    //    Microsoft::WRL::ComPtr<IUnknown> unkDevice = reinterpret_cast<IUnknown*>(*ppUnkDevice);
//
//    //    Microsoft::WRL::ComPtr<ID3D11Device> d3dDevice;
//    //    unkDevice.As(&d3dDevice);
//    //    LOG_FAILED(hr);
//
//    //    hr = d3dDevice.As(&this->d3dMultithread);
//    //    LOG_FAILED(hr);
//    //    if (SUCCEEDED(hr)) {
//    //        this->d3dMultithread->Enter();
//    //    }
//    //}
//    return hr;
//}
//
//HRESULT __stdcall MFDXGIDeviceManagerCustom::OpenDeviceHandle(HANDLE* phDevice) {
//    return mfDxgiDeviceManagerOrig->OpenDeviceHandle(phDevice);
//}
//
//HRESULT __stdcall MFDXGIDeviceManagerCustom::ResetDevice(IUnknown* pUnkDevice, UINT resetToken) {
//    return mfDxgiDeviceManagerOrig->ResetDevice(pUnkDevice, resetToken);
//}
//
//HRESULT __stdcall MFDXGIDeviceManagerCustom::TestDevice(HANDLE hDevice) {
//    return mfDxgiDeviceManagerOrig->TestDevice(hDevice);
//}
//
//HRESULT __stdcall MFDXGIDeviceManagerCustom::UnlockDevice(HANDLE hDevice, BOOL fSaveState) {
//    HRESULT hr = mfDxgiDeviceManagerOrig->UnlockDevice(hDevice, fSaveState);
//    //if (this->d3dMultithread) {
//    //    this->d3dMultithread->Leave();
//    //    this->d3dMultithread.Reset();
//    //}
//    ////LOG_DEBUG_D(L"UnlockDevice [th = ({}) | \"{}\"]", HH::GetThreadId(), HH::ThreadNameHelper::GetThreadName());
//    return hr;
//}
//
//
//namespace details {
//    const uint32_t DxDeviceParams::DefaultD3D11CreateFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
//
//    DxDeviceParams::DxDeviceParams()
//        : d3d11CreateFlags(0)
//    {}
//
//    DxDeviceParams::DxDeviceParams(uint32_t d3d11CreateFlags)
//        : d3d11CreateFlags(d3d11CreateFlags)
//    {}
//
//
//    DxDevice::DxDevice(const DxDeviceParams* params)
//        : featureLevel(D3D_FEATURE_LEVEL_9_1)
//    {
//        this->CreateDeviceIndependentResources();
//        this->CreateDeviceDependentResources(params);
//    }
//
//    DxDevice::~DxDevice() {
//        HRESULT hr = S_OK;
//        Microsoft::WRL::ComPtr<IDXGIDevice3> dxgiDev;
//
//        hr = this->d3dDev.As(&dxgiDev);
//
//        if (SUCCEEDED(hr)) {
//            // https://msdn.microsoft.com/en-us/library/windows/desktop/dn280346(v=vs.85).aspx
//            // apps should call ID3D11DeviceContext::ClearState before calling Trim
//            {
//                auto dxCtx = this->ctxSafeObj.Lock();
//                dxCtx->D3D()->ClearState();
//            }
//
//            dxgiDev->Trim();
//        }
//    }
//
//    DxDeviceCtxSafeObj_t::_Locked DxDevice::LockContext() const {
//        return this->ctxSafeObj.Lock();
//    }
//
//    D3D_FEATURE_LEVEL DxDevice::GetDeviceFeatureLevel() const {
//        return this->featureLevel;
//    }
//
//    void DxDevice::CreateDeviceIndependentResources() {
//        HRESULT hr = S_OK;
//
//        hr = DWriteCreateFactory(
//            DWRITE_FACTORY_TYPE_SHARED,
//            __uuidof(IDWriteFactory),
//            (IUnknown**)this->dwriteFactory.GetAddressOf());
//        H::System::ThrowIfFailed(hr);
//
//        hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_MULTI_THREADED, IID_PPV_ARGS(this->d2dFactory.GetAddressOf()));
//        H::System::ThrowIfFailed(hr);
//    }
//
//    void DxDevice::CreateDeviceDependentResources(const DxDeviceParams* params) {
//        HRESULT hr = S_OK;
//        uint32_t flags;
//        Microsoft::WRL::ComPtr<ID3D11DeviceContext> d3dCtxTmp;
//        Microsoft::WRL::ComPtr<ID3D11DeviceContext1> d3dCtx;
//        Microsoft::WRL::ComPtr<ID2D1DeviceContext> d2dCtx;
//
//        if (params) {
//            flags = params->d3d11CreateFlags;
//        }
//        else {
//            flags = DxDeviceParams::DefaultD3D11CreateFlags;
//        }
//
//#ifdef _DEBUG
//        if (DxDevice::SdkLayersAvailable()) {
//            flags |= D3D11_CREATE_DEVICE_DEBUG;
//        }
//#endif
//
//        D3D_FEATURE_LEVEL featureLevels[] = {
//            D3D_FEATURE_LEVEL_11_1,
//            D3D_FEATURE_LEVEL_11_0,
//            D3D_FEATURE_LEVEL_10_1,
//            D3D_FEATURE_LEVEL_10_0,
//            D3D_FEATURE_LEVEL_9_3,
//            D3D_FEATURE_LEVEL_9_2,
//            D3D_FEATURE_LEVEL_9_1
//        };
//
//        hr = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE,
//            nullptr, flags,
//            featureLevels, ARRAY_SIZE(featureLevels),
//            D3D11_SDK_VERSION,
//            this->d3dDev.GetAddressOf(), &this->featureLevel,
//            d3dCtxTmp.GetAddressOf());
//
//        if (FAILED(hr)) {
//            hr = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_WARP,
//                nullptr, flags,
//                featureLevels, ARRAY_SIZE(featureLevels),
//                D3D11_SDK_VERSION,
//                this->d3dDev.GetAddressOf(), &this->featureLevel,
//                d3dCtxTmp.GetAddressOf());
//            H::System::ThrowIfFailed(hr);
//        }
//
//        hr = d3dCtxTmp.As(&d3dCtx);
//        H::System::ThrowIfFailed(hr);
//
//        this->EnableD3DDeviceMultithreading();
//        this->CreateD2DDevice();
//
//        d2dCtx = this->CreateD2DDeviceContext();
//        this->d2dCtxMt = D2DCtxMt(d2dCtx);
//        //this->ctxSafeObj = std::make_unique<DxDeviceCtx>(d2dCtx, d3dCtx, this->d3dMultithread);
//        this->ctxSafeObj = std::make_unique<DxDeviceCtx>(d2dCtx, d3dCtx);
//    }
//
//    void DxDevice::EnableD3DDeviceMultithreading() {
//        HRESULT hr = S_OK;
//
//        hr = this->d3dDev.As(&this->d3dMultithread);
//        H::System::ThrowIfFailed(hr);
//
//        this->d3dMultithread->SetMultithreadProtected(TRUE);
//    }
//
//    void DxDevice::CreateD2DDevice() {
//        HRESULT hr = S_OK;
//        D2D1_CREATION_PROPERTIES creationProps;
//        Microsoft::WRL::ComPtr<IDXGIDevice> dxgiDev;
//
//        hr = this->d3dDev.As(&dxgiDev);
//        H::System::ThrowIfFailed(hr);
//
//#ifdef _DEBUG
//        creationProps.debugLevel = D2D1_DEBUG_LEVEL_INFORMATION;
//#else
//        creationProps.debugLevel = D2D1_DEBUG_LEVEL_NONE;
//#endif
//
//        creationProps.options = D2D1_DEVICE_CONTEXT_OPTIONS_ENABLE_MULTITHREADED_OPTIMIZATIONS;
//        creationProps.threadingMode = D2D1_THREADING_MODE_MULTI_THREADED;
//
//        /*
//        https://msdn.microsoft.com/query/dev14.query?appId=Dev14IDEF1&l=EN-US&k=k%28d2d1_1%2FD2D1CreateDevice%29;k%28D2D1CreateDevice%29;k%28DevLang-C%2B%2B%29;k%28TargetOS-Windows%29&rd=true
//        It's probably better to use D2DFactory::CreateDevice
//        to use same d2dFactory
//        */
//        hr = this->d2dFactory->CreateDevice(dxgiDev.Get(), this->d2dDevice.GetAddressOf());
//        H::System::ThrowIfFailed(hr);
//
//        /*hr = D2D1CreateDevice(dxgiDev.Get(), creationProps, this->d2dDevice.GetAddressOf());
//        H::System::ThrowIfFailed(hr);*/
//    }
//
//    Microsoft::WRL::ComPtr<ID2D1DeviceContext> DxDevice::CreateD2DDeviceContext() {
//        HRESULT hr = S_OK;
//        Microsoft::WRL::ComPtr<ID2D1DeviceContext> d2dCtx;
//
//        hr = this->d2dDevice->CreateDeviceContext(
//            D2D1_DEVICE_CONTEXT_OPTIONS_ENABLE_MULTITHREADED_OPTIMIZATIONS,
//            d2dCtx.GetAddressOf());
//        H::System::ThrowIfFailed(hr);
//
//        return d2dCtx;
//    }
//
//    bool DxDevice::SdkLayersAvailable() {
//        HRESULT hr = D3D11CreateDevice(
//            nullptr,
//            D3D_DRIVER_TYPE_NULL,       // There is no need to create a real hardware device.
//            0,
//            D3D11_CREATE_DEVICE_DEBUG,  // Check for the SDK layers.
//            nullptr,                    // Any feature level will do.
//            0,
//            D3D11_SDK_VERSION,          // Always set this to D3D11_SDK_VERSION for Windows Store apps.
//            nullptr,                    // No need to keep the D3D device reference.
//            nullptr,                    // No need to know the feature level.
//            nullptr                     // No need to keep the D3D device context reference.
//        );
//
//        return SUCCEEDED(hr);
//    }
//
//
//
//    DxDeviceMF::DxDeviceMF(const DxDeviceParams* params)
//        : DxDevice(params)
//    {}
//
//    const IID IID_IMFDXGIDeviceManager__;
//    DEFINE_GUID(IID_IMFDXGIDeviceManager__, 0xEB533D5D, 0x2DB6, 0x40F8, 0x97, 0xA9, 0x49, 0x46, 0x92, 0x01, 0x4F, 0x07);
//
//    void DxDeviceMF::CreateMFDXGIDeviceManager() {
//        if (this->mfDxgiDeviceManagerCustom) {
//            LOG_WARNING_D("mfDxgiDeviceManagerCustom already initialized, ignore");
//            return;
//        }
//        HRESULT hr = S_OK;
//
//        uint32_t resetToken = 0;
//        Microsoft::WRL::ComPtr<IMFDXGIDeviceManager> mfDxgiDeviceManager;
//        hr = MFCreateDXGIDeviceManager(&resetToken, mfDxgiDeviceManager.ReleaseAndGetAddressOf());
//        H::System::ThrowIfFailed(hr);
//
//        hr = mfDxgiDeviceManager->ResetDevice(this->GetD3DDevice(), resetToken);
//        H::System::ThrowIfFailed(hr);
//
//        hr = this->d3dDev->SetPrivateDataInterface(IID_IMFDXGIDeviceManager__, mfDxgiDeviceManager.Get());
//        H::System::ThrowIfFailed(hr);
//
//        this->mfDxgiDeviceManagerCustom = Microsoft::WRL::Make<MFDXGIDeviceManagerCustom>(mfDxgiDeviceManager);
//    }
//
//    Microsoft::WRL::ComPtr<IMFDXGIDeviceManager> DxDeviceMF::GetMFDXGIDeviceManager() {
//        return this->mfDxgiDeviceManagerCustom;
//    }
//
//
//
//    DxVideoDeviceMF::DxVideoDeviceMF()
//        : DxDeviceMF(&DxDeviceParams(DxDeviceParams::DefaultD3D11CreateFlags | D3D11_CREATE_DEVICE_VIDEO_SUPPORT))
//    {}
//
//
//
//    DxDeviceMFLock::DxDeviceMFLock(const std::unique_ptr<details::DxDeviceMF>& dxDeviceMf)
//        : mfDxgiDeviceManagerLock{ dxDeviceMf->GetMFDXGIDeviceManager() }
//        //: mfDxgiDeviceManagerLock{ nullptr }
//        , dxDeviceMf{ dxDeviceMf }
//    {
//        Microsoft::WRL::ComPtr<ID3D11Device> mfD3dDeviceTmp;
//        this->mfDxgiDeviceManagerLock.LockDevice(mfD3dDeviceTmp.GetAddressOf());
//        //this->dxDeviceMf->GetD3DMultithreadCPtr()->Enter();
//    }
//
//    DxDeviceMFLock::~DxDeviceMFLock() {
//        //this->dxDeviceMf->GetD3DMultithreadCPtr()->Leave();
//        this->mfDxgiDeviceManagerLock.UnlockDevice();
//    }
//}
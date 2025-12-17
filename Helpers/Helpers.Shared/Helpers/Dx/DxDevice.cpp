#include "DxDevice.h"
#include <Helpers/Dx/DxHelpers.h>
#include "ISwapChainPanel.h"

namespace HELPERS_NS {
    namespace Dx {
        namespace details {
            // This flag adds support for surfaces with a different color channel ordering
            // than the API default. It is required for compatibility with Direct2D.
            const uint32_t DxDeviceParams::DefaultD3D11CreateFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;

            DxDeviceParams::DxDeviceParams()
                : d3d11CreateFlags(0)
            {}

            DxDeviceParams::DxDeviceParams(uint32_t d3d11CreateFlags)
                : d3d11CreateFlags(d3d11CreateFlags)
            {}


            //
            // DxDevice
            //
            DxDevice::DxDevice(const std::optional<DxDeviceParams>& params)
                : featureLevel{ D3D_FEATURE_LEVEL_9_1 }
            {
                this->CreateDeviceIndependentResources();
                this->CreateDeviceDependentResources(params);
            }

            DxDevice::~DxDevice() {
                HRESULT hr = S_OK;
                Microsoft::WRL::ComPtr<IDXGIDevice3> dxgiDev;

                hr = this->d3dDevice.As(&dxgiDev);

                if (SUCCEEDED(hr)) {
                    // https://msdn.microsoft.com/en-us/library/windows/desktop/dn280346(v=vs.85).aspx
                    // apps should call ID3D11DeviceContext::ClearState before calling Trim
                    {
                        auto dxCtx = this->ctxSafeObj.Lock();
                        dxCtx->D3D()->ClearState();
                    }

                    dxgiDev->Trim();
                }
            }


            DxDeviceCtxSafeObj_t::_Locked DxDevice::LockContext() const {
                return this->ctxSafeObj.Lock();
            }

            D3D_FEATURE_LEVEL DxDevice::GetDeviceFeatureLevel() const {
                return this->featureLevel;
            }


            void DxDevice::CreateDxgiFactory() {
                HRESULT hr = CreateDXGIFactory1(IID_PPV_ARGS(dxgiFactory.ReleaseAndGetAddressOf()));
                H::System::ThrowIfFailed(hr);
            }

            Microsoft::WRL::ComPtr<H::Dx::ISwapChainPanel> DxDevice::GetAssociatedSwapChainPanel() const {
                //LOG_ASSERT(this->associatedSwapChainPanel, L"this->associatedSwapChainPanel is empty");
                if (!this->associatedSwapChainPanel) {
                    LOG_WARNING_D(L"this->associatedSwapChainPanel is empty");
                }
                return this->associatedSwapChainPanel;
            }

            void DxDevice::SetAssociatedSwapChainPanel(
                Passkey<H::Dx::SwapChainPanel*>,
                Microsoft::WRL::ComPtr<H::Dx::ISwapChainPanel> associatedSwapChainPanel)
            {
                LOG_ASSERT(!this->associatedSwapChainPanel, L"this->associatedSwapChainPanel is already set");
                this->associatedSwapChainPanel = associatedSwapChainPanel;
            }



            void DxDevice::CreateDeviceIndependentResources() {
                HRESULT hr = S_OK;

                // Initialize Direct2D resources.
                D2D1_FACTORY_OPTIONS options;
                ZeroMemory(&options, sizeof(D2D1_FACTORY_OPTIONS));

#if defined(_DEBUG)
                // If the project is in a debug build, enable Direct2D debugging via SDK Layers.
                options.debugLevel = D2D1_DEBUG_LEVEL_INFORMATION;
#endif

                // Initialize the Direct2D Factory.

                hr = D2D1CreateFactory(
                    D2D1_FACTORY_TYPE_MULTI_THREADED,
                    __uuidof(ID2D1Factory3),
                    &options,
                    &this->d2dFactory
                );
                H::System::ThrowIfFailed(hr);


                // Initialize the DirectWrite Factory.
                hr = DWriteCreateFactory(
                    DWRITE_FACTORY_TYPE_SHARED,
                    __uuidof(IDWriteFactory3),
                    &this->dwriteFactory
                );
                H::System::ThrowIfFailed(hr);

                // Initialize the Windows Imaging Component (WIC) Factory.
                hr = CoCreateInstance(
                    CLSID_WICImagingFactory2,
                    nullptr,
                    CLSCTX_INPROC_SERVER,
                    IID_PPV_ARGS(&this->wicFactory)
                );
                H::System::ThrowIfFailed(hr);
            }

            void DxDevice::CreateDeviceDependentResources(const std::optional<DxDeviceParams>& params) {
                uint32_t creationFlags = 0;
                if (params) {
                    creationFlags = params->d3d11CreateFlags;
                }
                else {
                    creationFlags = DxDeviceParams::DefaultD3D11CreateFlags;
                }

#ifdef _DEBUG
                if (H::Dx::SdkLayersAvailable()) {
                    creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
                }
#endif

                this->CreateDxgiFactory();

                // This array defines the set of DirectX hardware feature levels this app will support.
                // Note the ordering should be preserved.
                // Don't forget to declare your application's minimum required feature level in its
                // description.  All applications are assumed to support 9.1 unless otherwise stated.
                D3D_FEATURE_LEVEL featureLevels[] = {
                    D3D_FEATURE_LEVEL_12_1,
                    D3D_FEATURE_LEVEL_12_0,
                    D3D_FEATURE_LEVEL_11_1,
                    D3D_FEATURE_LEVEL_11_0,
                    D3D_FEATURE_LEVEL_10_1,
                    D3D_FEATURE_LEVEL_10_0,
                    D3D_FEATURE_LEVEL_9_3,
                    D3D_FEATURE_LEVEL_9_2,
                    D3D_FEATURE_LEVEL_9_1
                };

                // Create the Direct3D 11 API device object and a corresponding context.
                Microsoft::WRL::ComPtr<ID3D11Device> d3dDeviceTmp;
                Microsoft::WRL::ComPtr<ID3D11DeviceContext> d3dCtxTmp;

                HRESULT hr = S_OK;
                hr = D3D11CreateDevice(
                    nullptr,					// Specify nullptr to use the default adapter.
                    D3D_DRIVER_TYPE_HARDWARE,	// Create a device using the hardware graphics driver.
                    0,							// Should be 0 unless the driver is D3D_DRIVER_TYPE_SOFTWARE.
                    creationFlags,				// Set debug and Direct2D compatibility flags.
                    featureLevels,				// List of feature levels this app can support.
                    ARRAYSIZE(featureLevels),	// Size of the list above.
                    D3D11_SDK_VERSION,			// Always set this to D3D11_SDK_VERSION for Microsoft Store apps.
                    &d3dDeviceTmp,			    // Returns the Direct3D device created.
                    &this->featureLevel,	    // Returns feature level of device created.
                    &d3dCtxTmp					// Returns the device immediate context.
                );

                if (FAILED(hr)) {
                    // If the initialization fails, fall back to the WARP device.
                    // For more information on WARP, see: 
                    // https://go.microsoft.com/fwlink/?LinkId=286690
                    hr = D3D11CreateDevice(
                        nullptr,
                        D3D_DRIVER_TYPE_WARP, // Create a WARP device instead of a hardware device.
                        0,
                        creationFlags,
                        featureLevels,
                        ARRAYSIZE(featureLevels),
                        D3D11_SDK_VERSION,
                        &d3dDeviceTmp,
                        &this->featureLevel,
                        &d3dCtxTmp
                    );
                    H::System::ThrowIfFailed(hr);
                }

                // Store pointers to the Direct3D 11.3 API device and immediate context.
                hr = d3dDeviceTmp.As(&this->d3dDevice);
                H::System::ThrowIfFailed(hr);

                Microsoft::WRL::ComPtr<ID3D11DeviceContext3> d3dCtx;
                hr = d3dCtxTmp.As(&d3dCtx);
                H::System::ThrowIfFailed(hr);


                // Create the Direct2D device object and a corresponding context.
                Microsoft::WRL::ComPtr<IDXGIDevice> dxgiDev;
                hr = this->d3dDevice.As(&dxgiDev);
                H::System::ThrowIfFailed(hr);

                hr = this->d2dFactory->CreateDevice(dxgiDev.Get(), this->d2dDevice.GetAddressOf());
                H::System::ThrowIfFailed(hr);

                Microsoft::WRL::ComPtr<ID2D1DeviceContext> d2dCtx;
                hr = this->d2dDevice->CreateDeviceContext(
                    D2D1_DEVICE_CONTEXT_OPTIONS_ENABLE_MULTITHREADED_OPTIMIZATIONS,
                    d2dCtx.GetAddressOf()
                );
                H::System::ThrowIfFailed(hr);
                
                this->d2dCtxMt = D2DCtxMt(d2dCtx);
                this->ctxSafeObj = std::make_unique<DxDeviceCtx>(d2dCtx, d3dCtx);
                this->EnableD3DDeviceMultithreading();
            }

            void DxDevice::EnableD3DDeviceMultithreading() {
                HRESULT hr = S_OK;

                hr = this->d3dDevice.As(&this->d3dMultithread);
                H::System::ThrowIfFailed(hr);

                this->d3dMultithread->SetMultithreadProtected(TRUE);
            }


            //
            // DxDeviceMF
            //
            DxDeviceMF::DxDeviceMF(const std::optional<DxDeviceParams>& params)
                : DxDevice(params)
            {}

            const IID IID_IMFDXGIDeviceManager__;
            DEFINE_GUID(IID_IMFDXGIDeviceManager__, 0xEB533D5D, 0x2DB6, 0x40F8, 0x97, 0xA9, 0x49, 0x46, 0x92, 0x01, 0x4F, 0x07);

            void DxDeviceMF::CreateMFDXGIDeviceManager() {
                if (this->mfDxgiDeviceManagerCustom) {
                    LOG_WARNING_D("mfDxgiDeviceManagerCustom already initialized, ignore");
                    return;
                }
                HRESULT hr = S_OK;

                uint32_t resetToken = 0;
                Microsoft::WRL::ComPtr<IMFDXGIDeviceManager> mfDxgiDeviceManager;
                hr = MFCreateDXGIDeviceManager(&resetToken, mfDxgiDeviceManager.ReleaseAndGetAddressOf());
                H::System::ThrowIfFailed(hr);

                hr = mfDxgiDeviceManager->ResetDevice(this->GetD3DDevice().Get(), resetToken);
                H::System::ThrowIfFailed(hr);

                hr = this->d3dDevice->SetPrivateDataInterface(IID_IMFDXGIDeviceManager__, mfDxgiDeviceManager.Get());
                H::System::ThrowIfFailed(hr);

                this->mfDxgiDeviceManagerCustom = Microsoft::WRL::Make<MFDXGIDeviceManagerCustom>(mfDxgiDeviceManager);
            }

            Microsoft::WRL::ComPtr<IMFDXGIDeviceManager> DxDeviceMF::GetMFDXGIDeviceManager() {
                return this->mfDxgiDeviceManagerCustom;
            }


            //
            // DxVideoDeviceMF
            //
            DxVideoDeviceMF::DxVideoDeviceMF()
                : DxDeviceMF(DxDeviceParams(DxDeviceParams::DefaultD3D11CreateFlags | D3D11_CREATE_DEVICE_VIDEO_SUPPORT))
            {}


            //
            // DxDeviceMFLock
            //
            DxDeviceMFLock::DxDeviceMFLock(const std::unique_ptr<details::DxDeviceMF>& dxDeviceMf)
                //: mfDxgiDeviceManagerLock{ dxDeviceMf->GetMFDXGIDeviceManager() }
                : mfDxgiDeviceManagerLock{ nullptr }
                , dxDeviceMf{ dxDeviceMf }
            {
                Microsoft::WRL::ComPtr<ID3D11Device> mfD3dDeviceTmp;
                //this->mfDxgiDeviceManagerLock.LockDevice(mfD3dDeviceTmp.GetAddressOf());
                this->dxDeviceMf->GetD3DMultithread()->Enter();
            }

            DxDeviceMFLock::~DxDeviceMFLock() {
                this->dxDeviceMf->GetD3DMultithread()->Leave();
                //this->mfDxgiDeviceManagerLock.UnlockDevice();
            }
        }
    }
}
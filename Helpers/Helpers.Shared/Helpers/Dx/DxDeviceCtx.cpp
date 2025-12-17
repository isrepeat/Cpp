#include "DxDeviceCtx.h"
#include <utility>

namespace HELPERS_NS {
    namespace Dx {
        DxDeviceCtx::DxDeviceCtx() {
        }

        DxDeviceCtx::DxDeviceCtx(
            const Microsoft::WRL::ComPtr<ID2D1DeviceContext>& d2dCtx,
            const Microsoft::WRL::ComPtr<ID3D11DeviceContext3>& d3dCtx)
            : d2dCtx{ d2dCtx }
            , d3dCtx{ d3dCtx }
        {}

        DxDeviceCtx::DxDeviceCtx(const DxDeviceCtx& other)
            : d3dCtx{ other.d3dCtx }
            , d2dCtx{ other.d2dCtx }
        {}

        DxDeviceCtx::DxDeviceCtx(DxDeviceCtx&& other)
            : d3dCtx{ std::move(other.d3dCtx) }
            , d2dCtx{ std::move(other.d2dCtx) }
        {}

        DxDeviceCtx& DxDeviceCtx::operator=(const DxDeviceCtx& other) {
            if (this != &other) {
                this->d3dCtx = other.d3dCtx;
                this->d2dCtx = other.d2dCtx;
            }

            return *this;
        }

        DxDeviceCtx& DxDeviceCtx::operator=(DxDeviceCtx&& other) {
            if (this != &other) {
                this->d3dCtx = std::move(other.d3dCtx);
                this->d2dCtx = std::move(other.d2dCtx);
            }

            return *this;
        }

        DxDeviceCtx::~DxDeviceCtx() {
        }


        ID2D1DeviceContext* DxDeviceCtx::D2D() const {
            return this->d2dCtx.Get();
        }

        ID3D11DeviceContext3* DxDeviceCtx::D3D() const {
            return this->d3dCtx.Get();
        }


        DxDeviceCtxLock::DxDeviceCtxLock(const std::unique_ptr<DxDeviceCtx>& dxDeviceCtx)
            : mfDxgiDeviceManagerLock{ this->GetMFDXGIManagerLock(dxDeviceCtx) }
            , d3dMultithread{ this->GetD3DMultithread(dxDeviceCtx) }
        {
            Microsoft::WRL::ComPtr<ID3D11Device> mfD3dDeviceTmp;
            this->mfDxgiDeviceManagerLock.LockDevice(mfD3dDeviceTmp.GetAddressOf());
            //this->d3dMultithread->Enter();
        }

        DxDeviceCtxLock::~DxDeviceCtxLock() {
            //this->d3dMultithread->Leave();
            this->mfDxgiDeviceManagerLock.UnlockDevice();
        }

        HELPERS_NS::Dx::MFDXGIDeviceManagerLock DxDeviceCtxLock::GetMFDXGIManagerLock(const std::unique_ptr<DxDeviceCtx>& dxDeviceCtx) {
            HRESULT hr = S_OK;
            Microsoft::WRL::ComPtr<ID3D11Device> d3dDevice;
            dxDeviceCtx->D3D()->GetDevice(d3dDevice.ReleaseAndGetAddressOf());

            Microsoft::WRL::ComPtr<IMFDXGIDeviceManager> mfDxgiDeviceManager;
            d3dDevice.As(&mfDxgiDeviceManager);
            LOG_FAILED(hr);
            if (SUCCEEDED(hr)) {
                return HELPERS_NS::Dx::MFDXGIDeviceManagerLock{ mfDxgiDeviceManager };
            }
            return HELPERS_NS::Dx::MFDXGIDeviceManagerLock{ nullptr };
        }

        Microsoft::WRL::ComPtr<ID3D10Multithread> DxDeviceCtxLock::GetD3DMultithread(const std::unique_ptr<DxDeviceCtx>& dxDeviceCtx) {
            HRESULT hr = S_OK;
            Microsoft::WRL::ComPtr<ID3D11Device> d3dDevice;
            dxDeviceCtx->D3D()->GetDevice(d3dDevice.ReleaseAndGetAddressOf());

            Microsoft::WRL::ComPtr<ID3D10Multithread> d3dMultithread;
            hr = d3dDevice.As(&d3dMultithread);
            HELPERS_NS::System::ThrowIfFailed(hr);

            return d3dMultithread;
        }
    }
}
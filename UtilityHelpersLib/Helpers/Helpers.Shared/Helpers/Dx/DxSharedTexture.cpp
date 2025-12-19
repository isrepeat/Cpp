#include "DxSharedTexture.h"
#include <Helpers/System.h>
#include <Helpers/Memory.h>

namespace HELPERS_NS {
    namespace Dx {
        DxSharedTexture::DxSharedTexture(
            const D3D11_TEXTURE2D_DESC& desc,
            Microsoft::WRL::ComPtr<ID3D11Device> dstDevice,
            Microsoft::WRL::ComPtr<ID3D11Device> srcDevice) 
            : dstDevice{ dstDevice }
            , srcDevice{ srcDevice }
        {
            HRESULT hr = S_OK;
            auto descCopy = desc;

            descCopy.MiscFlags |= D3D11_RESOURCE_MISC_SHARED_KEYEDMUTEX | D3D11_RESOURCE_MISC_SHARED_NTHANDLE;

            hr = dstDevice->CreateTexture2D(&descCopy, nullptr, &this->dstDeviceTexture);
            HELPERS_NS::System::ThrowIfFailed(hr);

            Microsoft::WRL::ComPtr<IDXGIResource1> dxgiRes;
            hr = this->dstDeviceTexture.As(&dxgiRes);
            HELPERS_NS::System::ThrowIfFailed(hr);

            hr = this->dstDeviceTexture.As(&this->dstDeviceTextureMtx);
            HELPERS_NS::System::ThrowIfFailed(hr);

            hr = dxgiRes->CreateSharedHandle(
                nullptr,
                DXGI_SHARED_RESOURCE_READ | DXGI_SHARED_RESOURCE_WRITE,
                nullptr,
                HELPERS_NS::GetAddressOf(this->sharedTextureHandle));
            HELPERS_NS::System::ThrowIfFailed(hr);

            Microsoft::WRL::ComPtr<ID3D11Device1> srcDevice1;
            hr = srcDevice.As(&srcDevice1);
            HELPERS_NS::System::ThrowIfFailed(hr);

            hr = srcDevice1->OpenSharedResource1(this->sharedTextureHandle.get(), IID_PPV_ARGS(&this->srcDeviceTexture));
            HELPERS_NS::System::ThrowIfFailed(hr);

            hr = this->srcDeviceTexture.As(&this->srcDeviceTextureMtx);
            HELPERS_NS::System::ThrowIfFailed(hr);
        }

        DxSharedTexture::operator bool() const {
            return this->sharedTextureHandle != nullptr;
        }

        DxSharedTextureLocked DxSharedTexture::GetLockedTextureOnDstDevice() const {
            return DxSharedTextureLocked(this->dstDeviceTexture, this->dstDeviceTextureMtx);
        }
        DxSharedTextureLocked DxSharedTexture::GetLockedTextureOnSrcDevice() const {
            return DxSharedTextureLocked(this->srcDeviceTexture, this->srcDeviceTextureMtx);
        }
        //DxSharedTextureLocker DxSharedTexture::GetTextureOnSrcDevice() const {
        //    return DxSharedTextureLocker(this->dstDeviceTexture, this->dstDeviceTextureMtx);
        //}

        //DxSharedTextureLocker DxSharedTexture::GetTextureOnDstDevice() const {
        //    return DxSharedTextureLocker(this->srcDeviceTexture, this->srcDeviceTextureMtx);
        //}


        DxSharedTextureLocked DxSharedTexture::CopyFrom(
            const Microsoft::WRL::ComPtr<ID3D11Texture2D>& srcTexture)
        {
            auto textureOnSrcDeviceLocked = this->GetLockedTextureOnSrcDevice();

            Microsoft::WRL::ComPtr<ID3D11DeviceContext> srcDeviceContext;
            srcDevice->GetImmediateContext(srcDeviceContext.GetAddressOf());

            // Copy srcTexture that allocated on srcDevice to shared texture
            srcDeviceContext->CopyResource(textureOnSrcDeviceLocked.GetTexture(), srcTexture.Get());

            return this->GetLockedTextureOnDstDevice();
        }


        DxSharedTextureLocked::DxSharedTextureLocked(
            Microsoft::WRL::ComPtr<ID3D11Texture2D> tex,
            Microsoft::WRL::ComPtr<IDXGIKeyedMutex> texMtx) 
            : tex(tex)
            , texMtx(texMtx)
        {
            if (this->texMtx) {
                HRESULT hr = this->texMtx->AcquireSync(0, INFINITE);
                H::System::ThrowIfFailed(hr);
            }
        }

        DxSharedTextureLocked::DxSharedTextureLocked(DxSharedTextureLocked&& other) noexcept
            : tex{ std::move(other.tex) }
            , texMtx{ std::move(other.texMtx) } {
            other.tex.Reset();
            other.texMtx.Reset();
        }

        DxSharedTextureLocked& DxSharedTextureLocked::operator=(DxSharedTextureLocked&& other) noexcept {
            if (this != &other) {
                // release current lock if exists
                if (this->texMtx) {
                    HRESULT hr = this->texMtx->ReleaseSync(0);
                    H::System::ThrowIfFailed(hr);
                }
                this->tex = std::move(other.tex);
                this->texMtx = std::move(other.texMtx);
                other.tex.Reset();
                other.texMtx.Reset();
            }
            return *this;
        }

        DxSharedTextureLocked::~DxSharedTextureLocked() {
            if (this->texMtx) {
                HRESULT hr = this->texMtx->ReleaseSync(0);
                H::System::ThrowIfFailed(hr);
            }
        }

        ID3D11Texture2D* DxSharedTextureLocked::GetTexture() const {
            return this->tex.Get();
        }

        bool DxSharedTextureLocked::IsLocked() const {
            return this->texMtx != nullptr;
        }



        DxSharedTextureLocker::DxSharedTextureLocker(
            const Microsoft::WRL::ComPtr<ID3D11Texture2D>& tex,
            const Microsoft::WRL::ComPtr<IDXGIKeyedMutex>& texMtx)
            : tex(tex)
            , texMtx(texMtx)
        {}

        ID3D11Texture2D* DxSharedTextureLocker::GetTexture() const {
            assert(this->locked);
            return this->tex.Get();
        }

        void DxSharedTextureLocker::lock() {
            HRESULT hr = S_OK;

            hr = this->texMtx->AcquireSync(0, INFINITE);
            H::System::ThrowIfFailed(hr);

            this->locked = true;
        }

        void DxSharedTextureLocker::unlock() {
            HRESULT hr = S_OK;

            hr = this->texMtx->ReleaseSync(0);
            H::System::ThrowIfFailed(hr);

            this->locked = false;
        }
    }
}

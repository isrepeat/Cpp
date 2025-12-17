#pragma once
#include <Helpers/common.h>
#include <Helpers/UniqueHandle.h>
#include <d3d11_1.h>
#include <d3d11.h>
#include <wrl.h>

namespace HELPERS_NS {
    namespace Dx {
        class DxSharedTextureLocked;
        class DxSharedTextureLocker;

        // creates texture shared between 2 ID3D11Device's
        class DxSharedTexture {
        public:
            DxSharedTexture() = default;
            DxSharedTexture(
                const D3D11_TEXTURE2D_DESC& desc,
                Microsoft::WRL::ComPtr<ID3D11Device> dstDevice,
                Microsoft::WRL::ComPtr<ID3D11Device> srcDevice);

            explicit operator bool() const;

            DxSharedTextureLocked GetLockedTextureOnDstDevice() const;
            DxSharedTextureLocked GetLockedTextureOnSrcDevice() const;
            //DxSharedTextureLocker GetTextureOnSrcDevice() const;
            //DxSharedTextureLocker GetTextureOnDstDevice() const;

            void CopyTexture(
                ID3D11Texture2D** ppDstTexture,
                const Microsoft::WRL::ComPtr<ID3D11Texture2D>& srcTexture);

        private:
            Microsoft::WRL::ComPtr<ID3D11Device> dstDevice;
            Microsoft::WRL::ComPtr<ID3D11Texture2D> dstDeviceTexture;
            Microsoft::WRL::ComPtr<IDXGIKeyedMutex> dstDeviceTextureMtx;
            UHANDLE sharedTextureHandle;
            Microsoft::WRL::ComPtr<ID3D11Device> srcDevice;
            Microsoft::WRL::ComPtr<ID3D11Texture2D> srcDeviceTexture;
            Microsoft::WRL::ComPtr<IDXGIKeyedMutex> srcDeviceTextureMtx;
            
        };

        class DxSharedTextureLocked {
        public:
            DxSharedTextureLocked(
                Microsoft::WRL::ComPtr<ID3D11Texture2D> tex,
                Microsoft::WRL::ComPtr<IDXGIKeyedMutex> texMtx);
            ~DxSharedTextureLocked();

            ID3D11Texture2D* GetTexture() const;

        private:
            Microsoft::WRL::ComPtr<ID3D11Texture2D> tex;
            Microsoft::WRL::ComPtr<IDXGIKeyedMutex> texMtx;
        };

        // allows use texture only when texture mutex is locked
        class DxSharedTextureLocker {
        public:
            DxSharedTextureLocker() = default;
            DxSharedTextureLocker(
                const Microsoft::WRL::ComPtr<ID3D11Texture2D>& tex,
                const Microsoft::WRL::ComPtr<IDXGIKeyedMutex>& texMtx);

            ID3D11Texture2D* GetTexture() const;

            // https://en.cppreference.com/w/cpp/named_req/BasicLockable
            void lock();
            void unlock();

        private:
            bool locked = false;
            Microsoft::WRL::ComPtr<ID3D11Texture2D> tex;
            Microsoft::WRL::ComPtr<IDXGIKeyedMutex> texMtx;
        };
    }
}
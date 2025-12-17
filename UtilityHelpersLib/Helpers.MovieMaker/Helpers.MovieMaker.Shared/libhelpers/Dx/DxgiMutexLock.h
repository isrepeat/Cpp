#pragma once
#include "..\Macros.h"

#include <d3d11.h>
#include <dxgi1_2.h>
#include <cstdint>
#include <wrl.h>

class DxgiMutexLock {
public:
    NO_COPY(DxgiMutexLock);

    DxgiMutexLock();
    DxgiMutexLock(ID3D11Resource *res, uint64_t aquireKey = 0, uint64_t releaseKey = 0, DWORD aquireTime = INFINITE);
    DxgiMutexLock(ID3D11Texture2D *tex, uint64_t aquireKey = 0, uint64_t releaseKey = 0, DWORD aquireTime = INFINITE);
    DxgiMutexLock(DxgiMutexLock &&other);
    ~DxgiMutexLock();

    DxgiMutexLock &operator=(DxgiMutexLock &&other);

private:
    uint64_t releaseKey;
    Microsoft::WRL::ComPtr<IDXGIKeyedMutex> mtx;
};
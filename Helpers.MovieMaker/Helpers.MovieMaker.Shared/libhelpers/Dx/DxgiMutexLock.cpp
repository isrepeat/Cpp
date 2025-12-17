#include "pch.h"
#include "DxgiMutexLock.h"
#include "..\HSystem.h"

#include <utility>

DxgiMutexLock::DxgiMutexLock() {}

DxgiMutexLock::DxgiMutexLock(ID3D11Resource *res, uint64_t aquireKey, uint64_t releaseKey, DWORD aquireTime) {
    HRESULT hr = S_OK;

    hr = res->QueryInterface(IID_PPV_ARGS(this->mtx.GetAddressOf()));
    if (hr == E_NOINTERFACE) {
        this->mtx = nullptr;
    }
    else {
        H::System::ThrowIfFailed(hr);

        hr = this->mtx->AcquireSync(aquireKey, aquireTime);
        H::System::ThrowIfFailed(hr);
    }
}

DxgiMutexLock::DxgiMutexLock(ID3D11Texture2D *tex, uint64_t aquireKey, uint64_t releaseKey, DWORD aquireTime) {
    HRESULT hr = S_OK;

    hr = tex->QueryInterface(IID_PPV_ARGS(this->mtx.GetAddressOf()));
    if (hr == E_NOINTERFACE) {
        this->mtx = nullptr;
    }
    else {
        H::System::ThrowIfFailed(hr);

        hr = this->mtx->AcquireSync(aquireKey, aquireTime);
        H::System::ThrowIfFailed(hr);
    }
}

DxgiMutexLock::DxgiMutexLock(DxgiMutexLock &&other)
    : mtx(std::move(other.mtx)), releaseKey(std::move(other.releaseKey))
{}

DxgiMutexLock::~DxgiMutexLock() {
    if (this->mtx) {
        this->mtx->ReleaseSync(this->releaseKey);
    }
}

DxgiMutexLock &DxgiMutexLock::operator=(DxgiMutexLock &&other) {
    if (this != &other) {
        this->mtx = std::move(other.mtx);
        this->releaseKey = std::move(this->releaseKey);
    }

    return *this;
}
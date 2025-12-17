#include "pch.h"
#include "MFMediaBufferLock.h"
#include "libhelpers/HSystem.h"
#include "../UnlockableResource.h"

MFMediaBufferLock::MFMediaBufferLock() noexcept
    : mediaBuffer(nullptr)
    , mem(nullptr)
    , len(0)
    , maxLength(0)
{}

MFMediaBufferLock::MFMediaBufferLock(IMFMediaBuffer* buf)
    : mediaBuffer(buf)
{
    HRESULT hr = S_OK;

    hr = this->mediaBuffer->Lock(&this->mem, &this->maxLength, &this->len);
    H::System::ThrowIfFailed(hr);
}

MFMediaBufferLock::MFMediaBufferLock(MFMediaBufferLock&& other) noexcept
    : MFMediaBufferLock()
{
    swap(*this, other);
}

MFMediaBufferLock::~MFMediaBufferLock() {
    UnlockableResource::Noexcept(this, &MFMediaBufferLock::Unlock);
}

MFMediaBufferLock& MFMediaBufferLock::operator=(MFMediaBufferLock&& other) noexcept {
    swap(*this, other);
    return *this;
}

void swap(MFMediaBufferLock& a, MFMediaBufferLock& b) noexcept {
    using std::swap;
    swap(a.len, b.len);
    swap(a.maxLength, b.maxLength);
    swap(a.mediaBuffer, b.mediaBuffer);
    swap(a.mem, b.mem);
}

BYTE* MFMediaBufferLock::GetData() const noexcept {
    return this->mem;
}

uint32_t MFMediaBufferLock::GetLength() const noexcept {
    return this->len;
}

uint32_t MFMediaBufferLock::GetMaxLength() const noexcept {
    return this->maxLength;
}

void MFMediaBufferLock::Unlock() {
    UnlockableResource::Unlock(this, this->mediaBuffer, [&]
        {
            return this->mediaBuffer->Unlock();
        });
}
#pragma once
#include "MFInclude.h"

#include <cstdint>

class MFMediaBufferLock {
public:
    MFMediaBufferLock() noexcept;
    MFMediaBufferLock(IMFMediaBuffer* buf);
    MFMediaBufferLock(const MFMediaBufferLock&) = delete;
    MFMediaBufferLock(MFMediaBufferLock&& other) noexcept;
    ~MFMediaBufferLock();

    MFMediaBufferLock& operator=(const MFMediaBufferLock&) = delete;
    MFMediaBufferLock& operator=(MFMediaBufferLock&& other) noexcept;

    friend void swap(MFMediaBufferLock& a, MFMediaBufferLock& b) noexcept;

    BYTE* GetData() const noexcept;
    uint32_t GetLength() const noexcept;
    uint32_t GetMaxLength() const noexcept;

    void Unlock();

private:
    IMFMediaBuffer* mediaBuffer;
    BYTE* mem;
    DWORD len;
    DWORD maxLength;
};
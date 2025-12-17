#pragma once
#include "common.h"

namespace HELPERS_NS {
    template <typename T>
    class LocalPtr {
    public:
        //LocalRAII(int size)
        //	: pData{ (T)LocalAlloc(LPTR, size) }
        //{}

        LocalPtr() = default;

        LocalPtr(HLOCAL hPtr)
            : pData{ (T*)hPtr }
        {}
        ~LocalPtr() {
            if (pData != nullptr) {
                LocalFree((HLOCAL)pData);
            }
        }

        LocalPtr(const LocalPtr& x) = delete;
        LocalPtr(LocalPtr&& other) noexcept
            : pData{ other.pData }
        {
            other.pData = nullptr;
        }

        LocalPtr& operator=(const LocalPtr& other) = delete;
        LocalPtr& operator=(LocalPtr&& other)
        {
            if (&other == this)
                return *this;

            if (pData != nullptr) {
                LocalFree((HLOCAL)pData);
            }

            pData = other.pData;
            other.pData = nullptr;

            return *this;
        }
        T& operator*() const { return *pData; }
        T* operator->() const { return pData; }

        operator bool() {
            return static_cast<bool>(pData);
        }

        T* get() {
            return pData;
        }

        T** ReleaseAndGetAdressOf() {
            pData = nullptr;
            return &pData;
        }

    private:
        T* pData = nullptr;
    };
}

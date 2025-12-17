#pragma once
#include "common.h"

#include <Windows.h>

namespace HELPERS_NS {
    class SmartVARIANT {
    public:
        SmartVARIANT();
        SmartVARIANT(const SmartVARIANT& other);
        SmartVARIANT(SmartVARIANT&& other);
        ~SmartVARIANT();

        SmartVARIANT& operator=(const SmartVARIANT& other);
        SmartVARIANT& operator=(SmartVARIANT&& other);

        VARIANT* operator->();
        const VARIANT* operator->() const;

        VARIANT& Get();
        const VARIANT& Get() const;

        VARIANT* GetAddressOf();
        VARIANT* ReleaseAndGetAddressOf();

        void Release();

        void Swap(SmartVARIANT& other);

    private:
        VARIANT vt;
    };
}

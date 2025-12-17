#include "SmartVARIANT.h"
#include "System.h"

#include <utility>

namespace HELPERS_NS {
    SmartVARIANT::SmartVARIANT() {
        VariantInit(&this->vt);
    }

    SmartVARIANT::SmartVARIANT(const SmartVARIANT& other)
        : SmartVARIANT()
    {
        HRESULT hr = S_OK;

        hr = VariantCopy(&this->vt, &other.vt);
        H::System::ThrowIfFailed(hr);
    }

    SmartVARIANT::SmartVARIANT(SmartVARIANT&& other)
        : SmartVARIANT()
    {
        // other will have empty VARIANT
        // this will have other.vt
        this->Swap(other);
    }

    SmartVARIANT::~SmartVARIANT() {
        try {
            this->Release();
        }
        catch (...) {}
    }

    SmartVARIANT& SmartVARIANT::operator=(const SmartVARIANT& other) {
        if (this != &other) {
            auto copy = SmartVARIANT(other);
            this->Swap(copy);
        }

        return *this;
    }

    SmartVARIANT& SmartVARIANT::operator=(SmartVARIANT&& other) {
        if (this != &other) {
            this->Swap(other);
        }

        return *this;
    }

    VARIANT* SmartVARIANT::operator->() {
        return &this->vt;
    }

    const VARIANT* SmartVARIANT::operator->() const {
        return &this->vt;
    }

    VARIANT& SmartVARIANT::Get() {
        return this->vt;
    }

    const VARIANT& SmartVARIANT::Get() const {
        return this->vt;
    }

    VARIANT* SmartVARIANT::GetAddressOf() {
        return &this->vt;
    }

    VARIANT* SmartVARIANT::ReleaseAndGetAddressOf() {
        this->Release();
        return this->GetAddressOf();
    }

    void SmartVARIANT::Release() {
        HRESULT hr = S_OK;

        hr = VariantClear(&this->vt);
        H::System::ThrowIfFailed(hr);
    }

    void SmartVARIANT::Swap(SmartVARIANT& other) {
        std::swap(this->vt, other.vt);
    }
}

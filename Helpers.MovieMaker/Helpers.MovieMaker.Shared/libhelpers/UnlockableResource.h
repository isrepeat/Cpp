#pragma once
#include "HSystem.h"

#include <type_traits>

class UnlockableResource {
public:
    template<class ThisT, class UnlockConditionFieldT, class UnlockFnT>
    static void Unlock(ThisT* _this, UnlockConditionFieldT& unlockConditionField, UnlockFnT unlockFn) {
        if (!unlockConditionField) {
            return;
        }

        HRESULT hr = S_OK;

        hr = unlockFn();
        H::System::ThrowIfFailed(hr);

        unlockConditionField = UnlockConditionFieldT();

        typedef typename std::decay<ThisT>::type ThisCtor;
        *_this = ThisCtor();
    }

    template<class Fn>
    static void Noexcept(Fn fn) noexcept {
        try {
            fn();
        }
        catch (...) {}
    }

    template<class ThisT, class MemFnT>
    static void Noexcept(ThisT* _this, MemFnT memFn) noexcept {
        UnlockableResource::Noexcept([&]
            {
                (_this->*memFn)();
            });
    }
};
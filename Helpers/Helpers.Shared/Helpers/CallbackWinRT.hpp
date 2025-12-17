#pragma once
#include "common.h"

#if COMPILE_FOR_WINRT == 1
#include "Callback.hpp"

namespace HELPERS_NS {
    template<typename T, typename R, typename... Ts>
    class GenericWeakWinRTCallback : public ICallback<R, Ts...> {
    public:
        GenericWeakWinRTCallback(T^ instance, R(*callbackFn)(T^ instance, Ts... args))
            : instanceWeak(instance)
            , callbackFn(callbackFn)
        {
        }

        GenericWeakWinRTCallback(const GenericWeakWinRTCallback& other)
            : instanceWeak(other.instanceWeak)
            , callbackFn(other.callbackFn)
        {
        }

        GenericWeakWinRTCallback(GenericWeakWinRTCallback&& other)
            : instanceWeak(std::move(other.instanceWeak))
            , callbackFn(std::move(other.callbackFn))
        {
        }

        ~GenericWeakWinRTCallback() {}

        GenericWeakWinRTCallback& operator=(const GenericWeakWinRTCallback& other) {
            if (this != &other) {
                this->instanceWeak = other.instanceWeak;
                this->callbackFn = other.callbackFn;
            }
            return *this;
        }

        GenericWeakWinRTCallback& operator=(GenericWeakWinRTCallback&& other) {
            if (this != &other) {
                this->instanceWeak = std::move(other.instanceWeak);
                this->callbackFn = std::move(other.callbackFn);
            }
            return *this;
        }

        R Invoke(Ts... args) override {
            auto instance = instanceWeak.Resolve<T>();
            if (instance) {
                return this->callbackFn(instance, std::forward<Ts>(args)...);
            }
            return R();
        }

        ICallback* Clone() const override {
            GenericWeakWinRTCallback* clone = new GenericWeakWinRTCallback(*this);
            return clone;
        }

    private:
        Platform::WeakReference instanceWeak;
        R(*callbackFn)(T^ instance, Ts... args);
    };

    // <instance> will be wrapped into WeakReference
    template<typename T, typename R, typename... Ts>
    Callback<R, Ts...> MakeWinRTCallback(T^ instance, R(*callbackFn)(T^ instance, Ts... args)) {
        return Callback<R, Ts...>(std::make_unique<GenericWeakWinRTCallback<T, R, Ts...>>(instance, callbackFn));
    }
}
#endif
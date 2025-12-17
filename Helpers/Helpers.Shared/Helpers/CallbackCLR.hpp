#pragma once
#include "common.h"

#if COMPILE_FOR_CLR
#include "Callback.hpp"
#include <msclr\auto_gcroot.h>

namespace HELPERS_NS {
    template<typename T, typename R, typename... Ts>
    class GenericWeakGCRootCallback : public ICallback<R, Ts...> {
    public:
        using ICallbackBase = ICallback<R, Ts...>;

        GenericWeakGCRootCallback(T data, R(__clrcall* callbackFn)(T data, Ts... args))
            : data(gcnew System::WeakReference(data))
            , callbackFn(callbackFn)
        {
        }

        GenericWeakGCRootCallback(const GenericWeakGCRootCallback& other)
            : callbackFn(other.callbackFn)
        {
            System::WeakReference^ weakRef = other.data.get();

            if (weakRef) {
                this->data = gcnew System::WeakReference(weakRef->Target);
            }
        }

        GenericWeakGCRootCallback(GenericWeakGCRootCallback&& other)
            : data(std::move(other.data))
            , callbackFn(std::move(other.callbackFn))
        {
        }

        virtual ~GenericWeakGCRootCallback() {}

        GenericWeakGCRootCallback& operator=(const GenericWeakGCRootCallback& other) {
            if (this != &other) {
                T tmp;
                System::WeakReference^ weakRef = other.data.get();

                if (weakRef) {
                    this->data = gcnew System::WeakReference(tmp);
                }

                this->callbackFn = other.callbackFn;
            }

            return *this;
        }

        GenericWeakGCRootCallback& operator=(GenericWeakGCRootCallback&& other) {
            if (this != &other) {
                this->data = std::move(other.data);
                this->callbackFn = std::move(other.callbackFn);
            }

            return *this;
        }

        R Invoke(Ts... args) override {
            System::WeakReference^ weakRef = this->data.get();

            if (weakRef) {
                T tmp = dynamic_cast<T>(weakRef->Target);

                if (tmp) {
                    return this->callbackFn(tmp, std::forward<Ts>(args)...);
                }
            }

            return R();
        }

    ICallbackBase* Clone() const override {
        GenericWeakGCRootCallback* clone = new GenericWeakGCRootCallback(*this);
        return clone;
    }

    private:
        msclr::auto_gcroot<System::WeakReference^> data;
        R(__clrcall* callbackFn)(T data, Ts... args);
    };

    // <data> will be wrapped into WeakReference
    template<typename T, typename R, typename... Ts>
    Callback<R, Ts...> MakeWeakGCRootCallback(T data, R(__clrcall* callbackFn)(T data, Ts... args)) {
        auto ptr = new GenericWeakGCRootCallback<T, R, Ts...>(data, callbackFn);
        auto icallback = std::unique_ptr<GenericWeakGCRootCallback<T, R, Ts...>>(ptr);
        return Callback<R, Ts...>(std::move(icallback));
    }
}
#endif
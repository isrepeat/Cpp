#pragma once
#include "Helpers/common.h"

#if COMPILE_FOR_CLR
#include <memory>

namespace CLR {
    // based on #include <msclr/com/ptr.h>
    // do not use this-> because operator-> overloaded and somewhy used in this-> expression
    template<typename T>
    ref class UniquePtr {
    public:
        UniquePtr()
            : nativePtr(nullptr)
        {}

        UniquePtr(std::unique_ptr<T> uptr)
            : nativePtr(uptr.release())
        {}

        UniquePtr(UniquePtr% other)
            : nativePtr(other.nativePtr)
        {
            other.nativePtr = nullptr;
        }

        // finalizer, can be called by GC if ~UniquePtr() not used
        // https://learn.microsoft.com/en-us/cpp/dotnet/how-to-define-and-consume-classes-and-structs-cpp-cli?view=msvc-170#BKMK_Destructors_and_finalizers
        // https://learn.microsoft.com/en-us/dotnet/standard/garbage-collection/implementing-dispose#the-dispose-method
        !UniquePtr() {
            Reset();
        }

        // destructor, called by delete or Dispose
        ~UniquePtr() {
            Reset();
        }

        UniquePtr% operator=(UniquePtr% other) {
            // https://learn.microsoft.com/en-us/cpp/extensions/tracking-reference-operator-cpp-component-extensions?view=msvc-170
            if (this != % other) {
                delete nativePtr;
                nativePtr = other.nativePtr;
                other.nativePtr = nullptr;
            }

            return *this;
        }

        // https://learn.microsoft.com/en-us/cpp/extensions/tracking-reference-operator-cpp-component-extensions?view=msvc-170
        UniquePtr% operator=(std::unique_ptr<T> other) {
            delete nativePtr;
            nativePtr = other.release();

            return *this;
        }

        std::unique_ptr<T> Detach() {
            auto res = std::unique_ptr<T>(nativePtr);
            nativePtr = nullptr;
            return res;
        }

        void Reset() {
            delete nativePtr;
            nativePtr = nullptr;
        }

        T* operator->() {
            return nativePtr;
        }

        // for use in a conditional
        explicit operator bool() {
            return static_cast<bool>(nativePtr);
        }

    private:
        T* nativePtr = nullptr;
    };
}
#endif
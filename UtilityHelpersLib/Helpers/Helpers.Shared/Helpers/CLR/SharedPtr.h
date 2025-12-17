#pragma once
#include "Helpers/common.h"

#if COMPILE_FOR_CLR
#include <memory>

namespace CLR {
    // based on #include <msclr/com/ptr.h>
    // do not use this-> because operator-> overloaded and somewhy used in this-> expression
    template<typename T>
    ref class SharedPtr {
    public:
        SharedPtr()
            : nativePtr(nullptr)
        {}

        SharedPtr(std::shared_ptr<T> ptr)
            : nativePtr(new std::shared_ptr<T>(std::move(ptr)))
        {}

        SharedPtr(SharedPtr% other)
            : nativePtr(nullptr)
        {
            if (other.nativePtr) {
                nativePtr = new std::shared_ptr<T>();

                if (*other.nativePtr) {
                    nativePtr = *other.nativePtr;
                }
            }
        }

        // finalizer, can be called by GC if ~SharedPtr() not used
        // https://learn.microsoft.com/en-us/cpp/dotnet/how-to-define-and-consume-classes-and-structs-cpp-cli?view=msvc-170#BKMK_Destructors_and_finalizers
        // https://learn.microsoft.com/en-us/dotnet/standard/garbage-collection/implementing-dispose#the-dispose-method
        !SharedPtr() {
            Reset();
        }

        // destructor, called by delete or Dispose
        ~SharedPtr() {
            Reset();
        }

        SharedPtr% operator=(SharedPtr% other) {
            // https://learn.microsoft.com/en-us/cpp/extensions/tracking-reference-operator-cpp-component-extensions?view=msvc-170
            if (this != %other) {
                delete nativePtr;

                if (other.nativePtr) {
                    nativePtr = new std::shared_ptr<T>();

                    if (*other.nativePtr) {
                        nativePtr = *other.nativePtr;
                    }
                }
            }

            return *this;
        }

        // https://learn.microsoft.com/en-us/cpp/extensions/tracking-reference-operator-cpp-component-extensions?view=msvc-170
        SharedPtr% operator=(std::shared_ptr<T> ptr) {
            if (nativePtr && *nativePtr) {
                *nativePtr = std::move(ptr);
            }
            else {
                delete nativePtr;
                nativePtr = new std::shared_ptr<T>(std::move(ptr));
            }

            return *this;
        }

        std::shared_ptr<T> Detach() {
            if (!nativePtr) {
                return nullptr;
            }

            return std::move(*nativePtr);
        }

        void Reset() {
            delete nativePtr;
            nativePtr = nullptr;
        }

        std::shared_ptr<T> GetNative() {
            if (!nativePtr) {
                return nullptr;
            }

            return *nativePtr;
        }

        T* operator->() {
            return nativePtr->get();
        }

        // for use in a conditional
        explicit operator bool() {
            if (!nativePtr) {
                return false;
            }

            if (!(*nativePtr)) {
                return false;
            }

            return true;
        }

    private:
        std::shared_ptr<T>* nativePtr = nullptr;
    };
}
#endif
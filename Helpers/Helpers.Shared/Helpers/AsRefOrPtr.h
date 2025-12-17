#pragma once
#include "common.h"

namespace HELPERS_NS {
    template<typename T>
    class AsRefOrPtr {
    public:
        AsRefOrPtr(T& ref)
            : ref(ref)
        {}

        operator T* () {
            return &this->ref;
        }

        operator T& () {
            return this->ref;
        }

    private:
        T& ref;
    };
}

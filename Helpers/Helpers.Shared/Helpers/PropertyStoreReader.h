#pragma once
#include "common.h"
#include "PropertyStoreKeyRef.h"

#include <propsys.h>
#include <string>

namespace HELPERS_NS {
    // does not own IPropertyStore
    class PropertyStoreReader {
    public:
        PropertyStoreReader(IPropertyStore* propStore);

        template<typename T>
        std::wstring ReadWString(const T& key) const {
            return this->ReadWStringHelper(PropertyStoreKeyRef<T>(key).Get());
        }

    private:
        std::wstring ReadWStringHelper(const PROPERTYKEY& propKey) const;

        IPropertyStore* propStore = nullptr;
    };
}

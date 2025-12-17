#pragma once
#include "common.h"

#include <wtypes.h>
#include <Devpropdef.h>

namespace HELPERS_NS {
    template<typename T>
    class PropertyStoreKeyRef;

    template<>
    class PropertyStoreKeyRef<PROPERTYKEY> {
    public:
        PropertyStoreKeyRef(const PROPERTYKEY& propKeyRef)
            : propKeyRef(propKeyRef)
        {}

        const PROPERTYKEY& Get() const {
            return this->propKeyRef;
        }

    private:
        const PROPERTYKEY& propKeyRef;
    };

 #if COMPILE_FOR_DESKTOP
    template<>
    class PropertyStoreKeyRef<DEVPROPKEY> {
    public:
        PropertyStoreKeyRef(const DEVPROPKEY& devPropKeyRef)
            : devPropKeyRef(devPropKeyRef)
        {
            this->propKey.fmtid = this->devPropKeyRef.fmtid;
            this->propKey.pid = this->devPropKeyRef.pid;
        }

        const PROPERTYKEY& Get() const {
            return this->propKey;
        }

    private:
        const DEVPROPKEY& devPropKeyRef;
        PROPERTYKEY propKey = {};
    };
#endif
}

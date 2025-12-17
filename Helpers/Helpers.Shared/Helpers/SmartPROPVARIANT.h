#pragma once
#include "common.h"

#include <Windows.h>
#include <Propidl.h>

namespace HELPERS_NS {
    class SmartPROPVARIANT {
    public:
        SmartPROPVARIANT();
        SmartPROPVARIANT(const PROPVARIANT& other);
        SmartPROPVARIANT(PROPVARIANT&& other);
        SmartPROPVARIANT(const SmartPROPVARIANT& other);
        SmartPROPVARIANT(SmartPROPVARIANT&& other);
        ~SmartPROPVARIANT();

        SmartPROPVARIANT& operator=(SmartPROPVARIANT other);

        PROPVARIANT* operator->();
        const PROPVARIANT* operator->() const;

        PROPVARIANT* get();
        const PROPVARIANT* get() const;

        void swap(SmartPROPVARIANT& other);

    private:
        PROPVARIANT pv;
    };
}

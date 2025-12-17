#pragma once
#include "Metainfo\Signature.h"

#include <assert.h>

/*
based on
http://stackoverflow.com/questions/22822836/type-switch-construct-in-c11
*/

// Uses dynamic cast inside
template<class BaseClass, class FirstSubclass, class... RestOfSubclasses>
void typeswitch(BaseClass *base, FirstSubclass &&first, RestOfSubclasses &&... rest)
{
    if (typeswitchHelper(base, first)) {
        return;
    }
    else {
        typeswitch(base, rest ...);
    }
}

template<typename BaseClass>
void typeswitch(BaseClass *) {
    //assert(false);
}

template<typename Base, typename T>
bool typeswitchHelper(Base *base, T func) {
    typedef typename Signature<T>::A1 DestType;

    if (DestType dst = dynamic_cast<DestType>(base)) {
        func(dst);
        return true;
    }

    return false;
}
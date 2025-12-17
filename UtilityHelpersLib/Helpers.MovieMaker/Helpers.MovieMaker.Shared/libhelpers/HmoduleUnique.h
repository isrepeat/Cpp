#pragma once
#include "config.h"
#if HAVE_WINRT == 0
#include "Macros.h"

#include <Windows.h>

class HmoduleUnique {
public:
    NO_COPY(HmoduleUnique);

    HmoduleUnique();
    HmoduleUnique(HMODULE val);
    HmoduleUnique(HmoduleUnique &&other);
    ~HmoduleUnique();

    HmoduleUnique &operator=(HmoduleUnique &&other);
    operator bool() const;

    HMODULE get() const;

private:
    HMODULE val;

    void FreeVal();
};
#endif // HAVE_WINRT == 0

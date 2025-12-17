#include "pch.h"
#include "HmoduleUnique.h"

#if HAVE_WINRT == 0
HmoduleUnique::HmoduleUnique()
    : val(nullptr)
{}

HmoduleUnique::HmoduleUnique(HMODULE val)
    : val(val)
{}

HmoduleUnique::HmoduleUnique(HmoduleUnique &&other)
    : val(other.val)
{
    other.val = nullptr;
}

HmoduleUnique::~HmoduleUnique() {
    this->FreeVal();
}

HmoduleUnique &HmoduleUnique::operator=(HmoduleUnique &&other) {
    if (this != &other) {
        this->FreeVal();

        this->val = other.val;
        other.val = nullptr;
    }

    return *this;
}

HmoduleUnique::operator bool() const {
    return this->val != nullptr;
}

HMODULE HmoduleUnique::get() const {
    return this->val;
}

void HmoduleUnique::FreeVal() {
    if (this->val) {
        FreeLibrary(this->val);
    }
}
#endif // HAVE_WINRT == 0

#include "SmartPROPVARIANT.h"

#include <algorithm>

using namespace HELPERS_NS;

SmartPROPVARIANT::SmartPROPVARIANT() {
    PropVariantInit(&this->pv);
}

SmartPROPVARIANT::SmartPROPVARIANT(const PROPVARIANT &other) {
    PropVariantCopy(&this->pv, &other);
}

SmartPROPVARIANT::SmartPROPVARIANT(PROPVARIANT &&other)
    : pv(other)
{
    PropVariantInit(&other);
}

SmartPROPVARIANT::SmartPROPVARIANT(const SmartPROPVARIANT&other)
    : SmartPROPVARIANT(other.pv)
{}

SmartPROPVARIANT::SmartPROPVARIANT(SmartPROPVARIANT&&other)
    : SmartPROPVARIANT(std::move(other.pv))
{}

SmartPROPVARIANT::~SmartPROPVARIANT() {
    PropVariantClear(&this->pv);
}

SmartPROPVARIANT& SmartPROPVARIANT::operator=(SmartPROPVARIANT other) {
    this->swap(other);
    return *this;
}

PROPVARIANT* SmartPROPVARIANT::operator->() {
    return &this->pv;
}

const PROPVARIANT* SmartPROPVARIANT::operator->() const {
    return &this->pv;
}

PROPVARIANT* SmartPROPVARIANT::get() {
    return &this->pv;
}

const PROPVARIANT* SmartPROPVARIANT::get() const {
    return &this->pv;
}

void SmartPROPVARIANT::swap(SmartPROPVARIANT&other) {
    using std::swap;

    swap(this->pv, other.pv);
}

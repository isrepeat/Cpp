#include "pch.h"
#include "UniquePROPVARIANT.h"

#include <algorithm>

UniquePROPVARIANT::UniquePROPVARIANT() {
    PropVariantInit(&this->pv);
}

UniquePROPVARIANT::UniquePROPVARIANT(PROPVARIANT pv)
    : pv(pv)
{}

UniquePROPVARIANT::UniquePROPVARIANT(UniquePROPVARIANT &&other)
    : UniquePROPVARIANT()
{
    this->swap(other);
}

UniquePROPVARIANT::~UniquePROPVARIANT() {
    PropVariantClear(&this->pv);
}

UniquePROPVARIANT &UniquePROPVARIANT::operator=(UniquePROPVARIANT other) {
    this->swap(other);
    return *this;
}

PROPVARIANT *UniquePROPVARIANT::operator->() {
    return this->get();
}

const PROPVARIANT *UniquePROPVARIANT::operator->() const {
    return this->get();
}

PROPVARIANT *UniquePROPVARIANT::get() {
    return &this->pv;
}

const PROPVARIANT *UniquePROPVARIANT::get() const {
    return &this->pv;
}

void UniquePROPVARIANT::swap(UniquePROPVARIANT &other) {
    std::swap(this->pv, other.pv);
}
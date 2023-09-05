#pragma once

#include <memory>

template<template<class T, class ...Types> class PtrType, class T, class ...Types>
class AddressOfHelper {
public:
    AddressOfHelper(PtrType<T, Types...> &tmp)
        : tmp(tmp), ptr(nullptr) {
    }

    ~AddressOfHelper() {
        if (this->ptr) {
            this->tmp.reset(this->ptr);
        }
    }

    operator T**() {
        return &this->ptr;
    }

private:
    PtrType<T, Types...> &tmp;
    T *ptr;
};

template<template<class T, class ...Types> class PtrType, class T, class ...Types>
AddressOfHelper<PtrType, T, Types...> GetAddressOf(PtrType<T, Types...> &ptr) {
    return AddressOfHelper<PtrType, T, Types...>(ptr);
}
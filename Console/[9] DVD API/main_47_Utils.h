#pragma once
#include <iostream>
#include <oaidl.h>
#include <memory>
#include <Helpers/Helpers.h>


inline void __BreakIfFail(HRESULT hr) {
    if (FAILED(hr)) {
#ifdef _DEBUG
        __debugbreak();
#else
        wprintf(L"ERROR: last error = %s", H::GetLastErrorAsString().c_str());
#endif
    }
}

#define dbreak __debugbreak()
#define DBREAK_IF_FAIL(hr) __BreakIfFail(hr)


template<template<class T, class ...Types> class PtrType, class T, class ...Types>
class AddressOfHelper {
public:
    AddressOfHelper(PtrType<T, Types...>& tmp)
        : tmp(tmp), ptr(nullptr) {
    }

    ~AddressOfHelper() {
        if (this->ptr) {
            this->tmp.reset(this->ptr);
        }
    }

    operator T** () {
        return &this->ptr;
    }

private:
    PtrType<T, Types...>& tmp;
    T* ptr;
};

template<template<class T, class ...Types> class PtrType, class T, class ...Types>
AddressOfHelper<PtrType, T, Types...> GetAddressOf(PtrType<T, Types...>& ptr) {
    return AddressOfHelper<PtrType, T, Types...>(ptr);
}



struct SAFEARRAYDeleter {
    void operator()(SAFEARRAY* obj) {
        SafeArrayDestroy(obj);
    }
};

typedef std::unique_ptr<SAFEARRAY, SAFEARRAYDeleter> SAFEARRAYUnique;

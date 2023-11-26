#pragma once
#include <memory>

template<class T>
struct ICloneable {
    virtual std::unique_ptr<T> Clone() = 0;
    virtual std::shared_ptr<T> CloneShared() = 0;
};
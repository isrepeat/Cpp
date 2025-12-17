#pragma once

#include <unknwn.h>

template<class T, class Deleter>
class ComPtrArray {
    static_assert(std::is_base_of<IUnknown, T>::value, "T must inherit from IUnknown");
public:
    typedef T** iterator;

    ComPtrArray()
        : ptrArraySize(0), ptrArray(nullptr)
    {}

    ComPtrArray(Deleter deleter)
        : deleter(std::move(deleter)), ptrArraySize(0), ptrArray(nullptr)
    {}

    ComPtrArray(const ComPtrArray &) = delete;

    ComPtrArray(ComPtrArray &&other) {
        this->Swap(other);
    }

    ~ComPtrArray() {
        this->Delete();
    }

    ComPtrArray &operator=(ComPtrArray other) {
        this->Swap(other);
        return &this;
    }

    T *& operator[](size_t i) {
        return this->ptrArray[i];
    }

    void Swap(ComPtrArray &other) {
        using std::swap;

        swap(this->deleter, other.deleter);
        swap(this->ptrArraySize, other.ptrArraySize);
        swap(this->ptrArray, other.ptrArray);
    }

    uint32_t *GetAddressOfSize() {
        return &this->ptrArraySize;
    }

    T ***GetAddressOf() {
        return &this->ptrArray;
    }

    T ***ReleaseAndGetAddressOf() {
        this->Delete();
        return &this->ptrArray;
    }

    size_t size() const {
        return this->ptrArraySize;
    }

    iterator begin() {
        return this->ptrArray;
    }

    iterator end() {
        return this->ptrArray + this->ptrArraySize;
    }

private:
    Deleter deleter;
    uint32_t ptrArraySize;
    T * *ptrArray;

    void Delete() {
        for (uint32_t i = 0; i < this->ptrArraySize; i++) {
            if (this->ptrArray[i]) {
                auto tmp = this->ptrArray[i];
                this->ptrArray[i] = nullptr;

                tmp->Release();
            }
        }

        this->ptrArraySize = 0;

        if (this->ptrArray) {
            auto tmp = this->ptrArray;
            this->ptrArray = nullptr;

            this->deleter(tmp);
        }
    }
};

template<class T, template<class T> class Deleter>
ComPtrArray<T, Deleter<T>> MakeComArrayPtr() {
    return ComPtrArray<T, Deleter<T>>();
}
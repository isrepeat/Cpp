#pragma once

#include <memory>

namespace H {
    // non-owning ptr to support setting raw pointer to nullptr after move
    // copy supported as in raw pointers
    template<typename T>
    class movable_ptr {
    public:
        movable_ptr() = default;

        movable_ptr(T* rawPtr)
            : ptr(rawPtr)
        {}

        movable_ptr(const movable_ptr& other)
            : movable_ptr(other.get())
        {}

        movable_ptr(movable_ptr&& other)
            : ptr(std::move(other.ptr))
        {}

        ~movable_ptr() {
            // non-owning ptr
            this->release();
        }

        movable_ptr& operator=(const movable_ptr& other) {
            if (this != &other) {
                auto copy = movable_ptr(other);
                this->swap(copy);
            }

            return *this;
        }

        movable_ptr& operator=(movable_ptr&& other) {
            if (this != &other) {
                this->swap(other);
            }

            return *this;
        }

        movable_ptr& operator=(nullptr_t) noexcept {
            reset();
            return *this;
        }

        T* operator->() const noexcept {
            return this->ptr.get();
        }

        T* get() const noexcept {
            return this->ptr.get();
        }

        explicit operator bool() const noexcept {
            return static_cast<bool>(this->ptr);
        }

        T* release() noexcept {
            return this->ptr.release();
        }

        void reset(T* newPtr = nullptr) noexcept {
            this->ptr.reset(newPtr);
        }

        void swap(movable_ptr& other) noexcept {
            std::swap(this->ptr, other.ptr);
        }

    private:
        // non-owning ptr, dtor calls release to not delete pointer
        std::unique_ptr<T> ptr;
    };
}

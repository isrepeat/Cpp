#pragma once
#include <mutex>

template<class T, class MutexT>
class ThreadSafeObjectBase {
public:
    class Locked {
    public:
        Locked(MutexT& mtx, T& obj)
            : lk(mtx)
            , obj(obj)
        {}

        T* operator->() const {
            return &this->obj;
        }

        T& Get() const {
            return this->obj;
        }

    private:
        std::unique_lock<MutexT> lk;
        T& obj;
    };

    ThreadSafeObjectBase() = default;

    template<class... Args>
    ThreadSafeObjectBase(Args&&... args)
        : obj(std::forward<Args>(args)...)
    {}

    Locked Get() {
        return Locked(this->mtx, this->obj);
    }

private:
    MutexT mtx;
    T obj;
};

template<class T>
using ThreadSafeObject = ThreadSafeObjectBase<T, std::mutex>;
template<class T>
using ThreadSafeObjectRecusrive = ThreadSafeObjectBase<T, std::recursive_mutex>;

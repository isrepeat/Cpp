#pragma once
#include "..\Macros.h"
#include "..\Thread\critical_section.h"
#include "..\Thread\condition_variable.h"

#include <vector>

namespace containers {
    template<class T>
    class ObjectPoolMt {
    public:
        struct UsageScope {
        public:
            NO_COPY(UsageScope);

            T obj;

            UsageScope()
                : parent(nullptr)
            {}

            UsageScope(ObjectPoolMt *parent, T &&obj)
                : parent(parent), obj(std::move(obj))
            {}

            UsageScope(UsageScope &&other)
                : parent(std::move(other.parent)),
                obj(std::move(other.obj))
            {
                other.parent = nullptr;
            }

            ~UsageScope() {
                if (this->parent) {
                    this->parent->Add(std::move(this->obj));
                }
            }

            UsageScope &operator=(UsageScope &&other) {
                if (this != &other) {
                    this->parent = std::move(other.parent);
                    this->obj = std::move(other.obj);
                    other.parent = nullptr;
                }

                return *this;
            }

        private:
            ObjectPoolMt *parent;
        };

        ObjectPoolMt() {}
        ~ObjectPoolMt() {}

        void Add(T &&obj) {
            {
                thread::critical_section::scoped_lock lk(this->cs);
                this->pool.push_back(std::move(obj));
            }
            
            this->cv.notify();
        }

        UsageScope Get() {
            thread::critical_section::scoped_lock lk(this->cs);

            while (this->pool.empty()) {
                this->cv.wait(this->cs);
            }

            UsageScope scope(this, std::move(this->pool.back()));

            this->pool.pop_back();

            return scope;
        }

    private:
        thread::critical_section cs;
        thread::condition_variable cv;
        std::vector<T> pool;
    };
}
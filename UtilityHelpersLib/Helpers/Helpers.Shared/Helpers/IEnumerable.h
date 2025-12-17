#pragma once
#include "common.h"

#include <memory>
#include <iterator>

namespace HELPERS_NS {
    // all implementations must start before begining
    // This allows to do
    // while(IEnumerator.MoveNext()) {}
    // https://learn.microsoft.com/en-us/dotnet/api/system.collections.ienumerator.movenext?view=net-8.0
    // After an enumerator is created or after the Reset method is called, an enumerator is positioned before the first element of the collection,
    // and the first call to the MoveNext method moves the enumerator over the first element of the collection.
    template<typename T>
    class IEnumerator {
    public:
        virtual ~IEnumerator() = default;

        virtual T GetCurrent() const = 0;
        virtual bool MoveNext() = 0;
        virtual void Reset() = 0;
        virtual bool IsSame(const IEnumerator<T>& other) const = 0;
        virtual std::shared_ptr<IEnumerator<T>> Clone() const = 0;
    };

    template<typename T>
    class IEnumerableIterator {
    public:
        // https://www.internalpointers.com/post/writing-custom-iterators-modern-cpp
        using iterator_category = std::forward_iterator_tag;
        using difference_type = void;
        using value_type = T;

        IEnumerableIterator(std::shared_ptr<IEnumerator<T>> enumerator)
            : enumerator(std::move(enumerator))
        {}

        value_type operator*() const {
            return this->enumerator->GetCurrent();
        }

        // Prefix increment
        IEnumerableIterator<T>& operator++() {
            this->enumerator->MoveNext();
            return *this;
        }

        // Postfix increment
        IEnumerableIterator<T> operator++(int) {
            auto clone = IEnumerableIterator<T>(this->enumerator->Clone());
            ++(*this);
            return clone;
        }

        template<typename T>
        friend bool operator== (const IEnumerableIterator<T>& a, const IEnumerableIterator<T>& b) {
            return a.enumerator->IsSame(*b.enumerator);
        };

        template<typename T>
        friend bool operator!= (const IEnumerableIterator<T>& a, const IEnumerableIterator<T>& b) {
            return !(a == b);
        };

    private:
        std::shared_ptr<IEnumerator<T>> enumerator;
    };

    template<typename T>
    class IEnumerable {
    public:
        virtual ~IEnumerable() = default;

        virtual std::shared_ptr<IEnumerator<T>> GetBeginEnumerator() = 0;
        virtual std::shared_ptr<IEnumerator<T>> GetEndEnumerator() = 0;

        // same name as in C#
        std::shared_ptr<IEnumerator<T>> GetEnumerator() {
            return this->GetBeginEnumerator();
        }

        IEnumerableIterator<T> begin() {
            auto enumerator = this->GetBeginEnumerator();
            // move to first element
            enumerator->MoveNext();

            return IEnumerableIterator<T>(std::move(enumerator));
        }

        IEnumerableIterator<T> end() {
            return IEnumerableIterator<T>(this->GetEndEnumerator());
        }
    };
}

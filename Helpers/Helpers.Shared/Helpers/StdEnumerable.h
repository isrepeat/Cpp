#pragma once
#include "common.h"
#include "IEnumerable.h"

#include <stdexcept>

namespace HELPERS_NS {
    template<typename It>
    class StdEnumerator final : public IEnumerator<typename It::value_type> {
    public:
        using T = typename It::value_type;

        StdEnumerator(It begin, It end)
            : begin(begin)
            , end(std::move(end))
            , current(begin)
        {}

        StdEnumerator(const StdEnumerator& other) = default;

        T GetCurrent() const {
            if (this->current == this->end) {
                throw std::runtime_error("end");
            }

            if (!this->started) {
                throw std::runtime_error("need to call MoveNext first");
            }

            return *this->current;
        }

        bool MoveNext() {
            if (this->current == this->end) {
                return false;
            }

            if (!this->started) {
                // for iterating before begin
                this->started = true;
                return true;
            }

            ++this->current;

            if (this->current == this->end) {
                return false;
            }

            return true;
        }

        void Reset() {
            this->started = false;
            this->current = this->begin;
        }

        bool IsSame(const IEnumerator<T>& other) const {
            auto* otherPtr = dynamic_cast<const StdEnumerator<It>*>(&other);
            if (!otherPtr) {
                return false;
            }

            bool same = this->current == otherPtr->current;
            return same;
        }

        std::shared_ptr<IEnumerator<T>> Clone() const {
            return std::make_shared<StdEnumerator<It>>(*this);
        }

    private:
        bool started = false;
        It begin;
        It end;
        It current;
    };

    template<typename It>
    std::shared_ptr<IEnumerator<typename It::value_type>> MakeIEnumeratorStd(It begin, It end) {
        return std::make_shared<StdEnumerator<It>>(std::move(begin), std::move(end));
    }

    template<typename It>
    class StdEnumerable final : public IEnumerable<typename It::value_type> {
    public:
        using T = typename It::value_type;

        // collectionHolder can be collection itself or object that contains collection
        // collectionHolder is needed to make sure that collection is alive as long as StdEnumerable
        // Be careful with collectionHolder to not make circular dependencies
        StdEnumerable(It begin, It end, std::shared_ptr<const void> collectionHolder)
            : collectionHolder(std::move(collectionHolder))
            , begin(std::move(begin))
            , end(std::move(end))
        {}

        std::shared_ptr<IEnumerator<T>> GetBeginEnumerator() override {
            return MakeIEnumeratorStd(this->begin, this->end);
        }

        std::shared_ptr<IEnumerator<T>> GetEndEnumerator() override {
            return MakeIEnumeratorStd(this->end, this->end);
        }

    private:
        // can hold lifetime of parent that owns collection
        std::shared_ptr<const void> collectionHolder;
        It begin;
        It end;
    };

    template<typename It>
    std::shared_ptr<IEnumerable<typename It::value_type>> MakeIEnumerableStdHelper(It begin, It end, std::shared_ptr<const void> collectionHolder) {
        return std::make_shared<StdEnumerable<It>>(std::move(begin), std::move(end), std::move(collectionHolder));
    }

    template<typename CollectionT, typename CollectionParentT>
    auto MakeIEnumerableStd(CollectionT& stdCollection, std::shared_ptr<CollectionParentT> collectionParent) {
        using It = decltype(stdCollection.begin());
        return MakeIEnumerableStdHelper(stdCollection.begin(), stdCollection.end(), collectionParent);
    }

    template<typename CollectionT>
    auto MakeIEnumerableStdRefNoOwn(CollectionT& stdCollection) {
        return MakeIEnumerableStd(stdCollection, std::shared_ptr<void>());
    }

    template<typename CollectionT>
    auto MakeIEnumerableStd(std::shared_ptr<CollectionT> stdCollection) {
        return MakeIEnumerableStd(*stdCollection, stdCollection);
    }

    template<typename CollectionT>
    auto MakeIEnumerableStdCopy(CollectionT stdCollection) {
        struct Holder {
            Holder(CollectionT&& stdCollection)
                : stdCollection(std::move(stdCollection))
            {}

            CollectionT stdCollection;
        };

        auto holder = std::make_shared<Holder>(std::move(stdCollection));

        return MakeIEnumerableStd(holder->stdCollection, holder);
    }

    template<typename CollectionT, typename CollectionParentT>
    auto MakeIEnumerableStdCopy(CollectionT stdCollection, std::shared_ptr<CollectionParentT> collectionParentSharedPtr) {
        using HolderBase = std::shared_ptr<CollectionParentT>;

        struct Holder : public HolderBase {
            Holder(std::shared_ptr<CollectionParentT> collectionParentSharedPtr, CollectionT&& stdCollection)
                : HolderBase(collectionParentSharedPtr)
                , stdCollection(std::move(stdCollection))
            {}

            CollectionT stdCollection;
        };

        auto holder = std::make_shared<Holder>(std::move(collectionParentSharedPtr), std::move(stdCollection));

        return MakeIEnumerableStd(holder->stdCollection, holder);
    }
}

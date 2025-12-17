#pragma once
#include "common.h"
#include "IObservableCollection.h"
#include "WeakEvent.h"

#include <vector>

namespace HELPERS_NS {
    template<typename T>
    class ObservableCollection final : public IObservableCollection<T> {
    public:
        IWeakEvent<const ObservableCollectionChangedArgs<T>&>& GetOnChangedEvent() const override {
            return this->onChangedEvent;
        }

        size_t GetCount() override {
            return this->items.size();
        }

        T GetAt(size_t idx) override {
            return this->items[idx];
        }

        void Add(T item) override {
            this->items.push_back(std::move(item));

            ObservableCollectionChangedArgs<T> changedArgs;

            changedArgs.action = ObservableCollectionAction::Add;
            changedArgs.newItems.push_back(this->items.back());
            changedArgs.newStartingIndex = this->items.size() - 1;

            this->onChangedEvent(changedArgs);
        }

        void Remove(T item) override {
            auto it = std::find(this->items.begin(), this->items.end(), item);
            if (it == this->items.end()) {
                return;
            }

            ObservableCollectionChangedArgs<T> changedArgs;

            changedArgs.action = ObservableCollectionAction::Remove;
            changedArgs.oldItems.push_back(*it);
            changedArgs.oldStartingIndex = static_cast<size_t>(std::distance(this->items.begin(), it));

            this->onChangedEvent(changedArgs);

            this->items.erase(it);
        }

        const auto& GetItems() const {
            return this->items;
        }

    private:
        mutable WeakEvent<const ObservableCollectionChangedArgs<T>&> onChangedEvent;
        std::vector<T> items;
    };
}

#pragma once
#include "common.h"
#include "IWeakEvent.h"

#include <vector>

namespace HELPERS_NS {
    enum ObservableCollectionAction {
        Unknown,
        Add,
        Remove
    };

    // https://learn.microsoft.com/en-us/dotnet/api/system.collections.specialized.notifycollectionchangedeventargs?view=net-8.0
    template<typename T>
    struct ObservableCollectionChangedArgs {
        ObservableCollectionAction action = ObservableCollectionAction::Unknown;
        std::vector<T> newItems;
        size_t newStartingIndex = 0;
        std::vector<T> oldItems;
        size_t oldStartingIndex = 0;
    };

    // https://learn.microsoft.com/en-us/dotnet/api/system.collections.objectmodel.observablecollection-1?view=net-8.0
    template<typename T>
    class IObservableCollection {
    public:
        virtual ~IObservableCollection() = default;

        virtual IWeakEvent<const ObservableCollectionChangedArgs<T>&>& GetOnChangedEvent() const = 0;
        virtual size_t GetCount() = 0;
        virtual T GetAt(size_t idx) = 0;

        // Adds an object to the end
        virtual void Add(T item) = 0;

        // Removes the first occurrence of a specific object
        virtual void Remove(T item) = 0;
    };
}

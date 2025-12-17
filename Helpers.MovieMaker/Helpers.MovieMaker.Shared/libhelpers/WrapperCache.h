#pragma once
#include "Thread\critical_section.h"

#include <map>
#include <memory>
#include <unordered_map>
#include <type_traits>
#include <libhelpers\raw_ptr.h>

template<class D, class S>
struct DynamicCastHelper {
    typedef S Src;
    typedef D Dst;

    static D Make(S arg) {
        D res = dynamic_cast<D>(arg);
        return res;
    }
};

template<class D, class S>
struct DynamicCastHelper<D, raw_ptr<S>> {
    typedef typename std::remove_pointer<S>::type SClean;
    typedef typename std::remove_pointer<D>::type DClean;

    typedef typename std::add_pointer<DClean>::type DPtr;

    typedef raw_ptr<SClean> Src;
    typedef raw_ptr<DClean> Dst;

    static Dst Make(Src arg) {
        Dst res = dynamic_cast<DPtr>(arg.get());
        return res;
    }
};

template<class D, class S>
struct DynamicCastHelper<D, std::shared_ptr<S>> {
    typedef std::shared_ptr<S> Src;
    typedef std::shared_ptr<D> Dst;

    static Dst Make(const Src &arg) {
        Dst res = std::dynamic_pointer_cast<D>(arg);
        return res;
    }
};

template<class D, class S>
typename DynamicCastHelper<D, S>::Dst MakeDynamicCast(S arg) {
    return DynamicCastHelper<D, S>::Make(arg);
}

template<bool Sync>
struct WrapperCacheSync {
    typedef thread::critical_section CriticalSection;
    typedef thread::critical_section::scoped_lock ScopedLock;
};

template<>
struct WrapperCacheSync<false> {
    struct CriticalSection {};
    struct ScopedLock {
        ScopedLock(const CriticalSection &cs) {}
    };
};

template<class DstTypePrivate, class DstTypePublic = DstTypePrivate>
class WrapperCacheBase {
public:
    WrapperCacheBase() {}
    virtual ~WrapperCacheBase() {}

protected:
    virtual DstTypePublic GetPublicWrapper(DstTypePrivate &wrapper) = 0;
};

template<class DstTypePrivate>
class WrapperCacheBase<DstTypePrivate, DstTypePrivate> {
public:
    WrapperCacheBase() {}
    virtual ~WrapperCacheBase() {}

protected:
    DstTypePrivate &GetPublicWrapper(DstTypePrivate &wrapper) {
        return wrapper;
    }
};

/*
InitData - data for wrapper initialization
SrcType - type that need to be wrapped
DstTypePrivate - private wrapper type
DstTypePublic - public wrapper type that available to user
Synchronized - whether to use critical section or not
LessOp - less comparison for SrcType
*/
template<class InitData, class SrcType, class DstTypePrivate, class DstTypePublic = DstTypePrivate, bool Synchronized = true, class LessOp = std::less<SrcType>>
class WrapperCache : public WrapperCacheBase<DstTypePrivate, DstTypePublic> {
public:
    WrapperCache()
        : canCreateWrappers(true)
    {}
    virtual ~WrapperCache() {}

    DstTypePublic Get(SrcType input, InitData initData) {
        DstTypePublic wrapper;
        typename WrapperCacheSync<Synchronized>::ScopedLock lk(this->cacheCs);

        if (this->canCreateWrappers) {
            auto find = this->cache.find(input);
            if (find != this->cache.end()) {
                wrapper = this->GetPublicWrapper(find->second);
            }
            else {
                DstTypePrivate newWrapper = this->CreateWrapper(input, initData);

                wrapper = this->GetPublicWrapper(newWrapper);

                this->cache.insert(std::make_pair(input, std::move(newWrapper)));
            }
        }

        return wrapper;
    }

    DstTypePublic FindWrapper(SrcType input) {
        DstTypePublic wrapper;
        typename WrapperCacheSync<Synchronized>::ScopedLock lk(this->cacheCs);

        if (this->canCreateWrappers) {
            auto find = this->cache.find(input);
            if (find != this->cache.end()) {
                wrapper = this->GetPublicWrapper(find->second);
            }
        }

        return wrapper;
    }

    void Remove(SrcType input) {
        typename WrapperCacheSync<Synchronized>::ScopedLock lk(this->cacheCs);

        auto find = this->cache.find(input);
        if (find != this->cache.end()) {
            this->cache.erase(find);
        }
    }

    bool GetCanCreateWrappers() {
        typename WrapperCacheSync<Synchronized>::ScopedLock lk(this->cacheCs);
        return this->canCreateWrappers;
    }

    void SetCanCreateWrappers(bool v) {
        typename WrapperCacheSync<Synchronized>::ScopedLock lk(this->cacheCs);
        this->canCreateWrappers = v;
    }

    void Clear() {
        typename WrapperCacheSync<Synchronized>::ScopedLock lk(this->cacheCs);
        this->cache.clear();
    }

    template<class Fn>
    void ForEach(Fn fn) {
        typename WrapperCacheSync<Synchronized>::ScopedLock lk(this->cacheCs);

        for (auto &i : this->cache) {
            if (!fn(i.first, i.second)) {
                break;
            }
        }
    }

    template<class WrapperT>
    typename DynamicCastHelper<WrapperT, DstTypePublic>::Dst Get(SrcType input, InitData initData) {
        typename DynamicCastHelper<WrapperT, DstTypePublic>::Dst wrapper = MakeDynamicCast<WrapperT>(this->Get(input, initData));
        return wrapper;
    }

protected:
    virtual DstTypePrivate CreateWrapper(SrcType input, InitData initData) = 0;

private:
    typename WrapperCacheSync<Synchronized>::CriticalSection cacheCs;
    std::map<SrcType, DstTypePrivate, LessOp> cache;
    bool canCreateWrappers; // used to stop creating wrappers before clear
};
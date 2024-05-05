#include <Helpers/ThreadSafeObject.hpp>
#include <functional>

namespace HH {
    //
     // LockedObjBase
     // NOTE: CreatorT::MutexT must be declared as mutable. 
     //       Keep other "LockedObj..." constructors params as const-members 
     //       to allow make "LockedObj" object from CreatorT with non-const members.
     //
    template <typename CreatorT>
    struct LockedObjBase {
        static constexpr std::string_view templateNotes = "Primary template";
    };


    template <template<typename, typename, typename> class CreatorT, typename MutexT, typename ObjT, typename CustomLockableT>
    struct LockedObjBase<CreatorT<MutexT, ObjT, CustomLockableT>>
    {
        static constexpr std::string_view templateNotes = "Specialized for <CreatorT<MutexT, ObjT, CustomLockableT>";

        _Acquires_lock_(this->lk); // suppress warning - "C26115: Falling release lock 'mx'"
        LockedObjBase(MutexT& mx, ObjT& obj, const CreatorT<MutexT, ObjT, CustomLockableT>* creator)
            : lk{ mx }
            , creator{ creator }
            , customLockableObj{ obj }
        {}

        _Releases_lock_(this->lk);
        ~LockedObjBase() {
        }

        const CreatorT<MutexT, ObjT, CustomLockableT>* GetCreator() const {
            return this->creator;
        }

    private:
        std::unique_lock<MutexT> lk;
        const CreatorT<MutexT, ObjT, CustomLockableT>* creator;
        CustomLockableT customLockableObj;
    };


    template <template<typename, typename, typename> class CreatorT, typename MutexT, typename ObjT>
    struct LockedObjBase<CreatorT<MutexT, ObjT, void>>
    {
        static constexpr std::string_view templateNotes = "Specialized for <CreatorT<MutexT, ObjT, void>";
        LockedObjBase(MutexT& mx, ObjT& /*obj*/, const CreatorT<MutexT, ObjT, void>* creator)
            : lk{ mx }
            , creator{ creator }
        {}

        const CreatorT<MutexT, ObjT, void>* GetCreator() const {
            return this->creator;
        }

    private:
        std::unique_lock<MutexT> lk;
        const CreatorT<MutexT, ObjT, void>* creator;
    };


    //
    // LockedObj
    //
    template <typename CreatorT>
    struct LockedObj {
        static constexpr std::string_view templateNotes = "Primary template";
    };


    template <template<typename, typename, typename> class CreatorT, typename MutexT, typename ObjT, typename CustomLockableT>
    struct LockedObj<CreatorT<MutexT, ObjT, CustomLockableT>>
        : LockedObjBase<CreatorT<MutexT, ObjT, CustomLockableT>>
    {
        static constexpr std::string_view templateNotes = "Specialized for <CreatorT<MutexT, ObjT, CustomLockableT>>";
        using _MyBase = LockedObjBase<CreatorT<MutexT, ObjT, CustomLockableT>>;

        LockedObj(
            MutexT& mx,
            //std::remove_const_t<ObjT>& obj,
            ObjT& obj,
            const CreatorT<MutexT, ObjT, CustomLockableT>* creator)
            : _MyBase(mx, obj, creator)
            , obj{ obj } // [by design]
        {}

        ~LockedObj() {
        }

        ObjT* operator->() const {
            return &this->obj;
        }
        ObjT& Get() const {
            return this->obj;
        }

    private:
        std::remove_const_t<ObjT>& obj;
    };


    //template<template<typename, typename, typename> class CreatorT, typename MutexT, typename ObjT, typename... Args, typename CustomLockableT>
    //struct LockedObj<CreatorT<MutexT, std::unique_ptr<ObjT, Args...>, CustomLockableT>>
    //    : LockedObjBase<CreatorT<MutexT, std::unique_ptr<ObjT, Args...>, CustomLockableT>>
    //{
    //    static constexpr std::string_view templateNotes = "Specialized for <MutexT, std::unique_ptr<ObjT, Args...>, CustomLockableT>";
    //    using _MyBase = LockedObjBase<CreatorT<MutexT, std::unique_ptr<ObjT, Args...>, CustomLockableT>>;

    //    LockedObj(
    //        MutexT& mx,
    //        const std::unique_ptr<ObjT, Args...>& obj,
    //        const CreatorT<MutexT, std::unique_ptr<ObjT, Args...>, CustomLockableT>* creator)
    //        : _MyBase(mx, obj, creator)
    //        , obj{ const_cast<std::unique_ptr<ObjT, Args...>&>(obj) } // [by design]
    //    {}

    //    ~LockedObj() {
    //    }

    //    std::unique_ptr<ObjT, Args...>& operator->() const {
    //        return this->obj;
    //    }
    //    std::unique_ptr<ObjT, Args...>& Get() const {
    //        return this->obj;
    //    }

    //private:
    //    std::unique_ptr<ObjT, Args...>& obj;
    //};


    //
    // ThreadSafeObject
    //
    template <typename MutexT, typename ObjT, typename CustomLockableT = void>
    class ThreadSafeObject {
    public:
        static constexpr std::string_view templateNotes = "Primary temlpate";
        using _Locked = LockedObj<ThreadSafeObject<MutexT, ObjT, CustomLockableT>>;
        using _LockedConst = LockedObj<ThreadSafeObject<MutexT, const ObjT, CustomLockableT>>;

        ThreadSafeObject()
            : obj{}
        {}

        template <typename... Args>
        ThreadSafeObject(Args&&... args)
            : obj{ std::forward<Args>(args)... }
        {}

        ThreadSafeObject(ThreadSafeObject& other) = delete;
        ThreadSafeObject& operator=(ThreadSafeObject& other) = delete;

        template<typename OtherT, std::enable_if_t<std::is_convertible_v<OtherT, ObjT>, int> = 0>
        ThreadSafeObject(OtherT&& otherObj)
            : obj{ std::move(otherObj) }
        {}

        template<typename OtherT, std::enable_if_t<std::is_convertible_v<OtherT, ObjT>, int> = 0>
        ThreadSafeObject& operator=(OtherT&& otherObj) {
            if (&this->obj != &otherObj) {
                this->obj = std::move(otherObj);
            }
            return *this;
        }

        _Locked Lock() {
            return _Locked{ this->mx, this->obj, this };
        }

        const _LockedConst LockConst() const {
            return _LockedConst{ this->mx, this->obj, this };
        }

    private:
        mutable MutexT mx;
        ObjT obj;
    };


    //template <typename MutexT, typename ObjT, typename CustomLockableT>
    //class ThreadSafeObject<MutexT, std::unique_ptr<ObjT>, CustomLockableT> {
    //public:
    //    static constexpr std::string_view templateNotes = "Specialized for <MutexT, std::unique_ptr<ObjT>, CustomLockableObj>";
    //    using _Locked = LockedObj<ThreadSafeObject<MutexT, std::unique_ptr<ObjT>, CustomLockableT>>;
    //    using _LockedConst = LockedObj<ThreadSafeObject<MutexT, const std::unique_ptr<ObjT>, CustomLockableT>>;

    //    ThreadSafeObject()
    //        : obj{ std::make_unique<ObjT>() }
    //    {}

    //    template <typename... Args>
    //    ThreadSafeObject(Args&&... args)
    //        : obj{ std::make_unique<ObjT>(std::forward<Args>(args)...) }
    //    {}

    //    ThreadSafeObject(ThreadSafeObject& other) = delete;
    //    ThreadSafeObject& operator=(ThreadSafeObject& other) = delete;

    //    template <typename OtherT, std::enable_if_t<std::is_convertible_v<std::unique_ptr<OtherT>, std::unique_ptr<ObjT>>, int> = 0>
    //    ThreadSafeObject(std::unique_ptr<OtherT>&& otherObj)
    //        : obj{ std::move(otherObj) }
    //    {}

    //    template <typename OtherT, std::enable_if_t<std::is_convertible_v<std::unique_ptr<OtherT>, std::unique_ptr<ObjT>>, int> = 0>
    //    ThreadSafeObject& operator=(std::unique_ptr<OtherT>&& otherObj) {
    //        if (this->obj.get() != otherObj.get()) {
    //            this->obj = std::move(otherObj);
    //        }
    //        return *this;
    //    }

    //    _Locked Lock() {
    //        return _Locked{ this->mx, this->obj, this };
    //    }

    //    const _LockedConst Lock() const {
    //        return _LockedConst{ this->mx, this->obj, this };
    //    }

    //private:
    //    mutable MutexT mx;
    //    std::unique_ptr<ObjT> obj;
    //};
}




class DxDeviceCtx {
public:
    DxDeviceCtx() = default;
    ~DxDeviceCtx() = default;

    void DrawD2D() {
    }

    void DrawD2DConst() const {
    }

private:
    int data;
};


//class DxDeviceCtxLock {
//public:
//    DxDeviceCtxLock(const std::unique_ptr<DxDeviceCtx>&) {}
//    ~DxDeviceCtxLock() {}
//};
//
////using DxDeviceCtxSafeObj_t = HH::ThreadSafeObject<std::recursive_mutex, std::unique_ptr<DxDeviceCtx>, DxDeviceCtxLock>;
//using DxDeviceCtxSafeObj_t = HH::ThreadSafeObject<std::recursive_mutex, DxDeviceCtx, DxDeviceCtxLock>;
//
//
//
//class DxLayerStack {
//public:
//    DxLayerStack(DxDeviceCtxSafeObj_t* dxCtxSafeObj)
//        : dxCtxSafeObj{ dxCtxSafeObj }
//    {}
//
//    void BeginDraw() {
//        auto ctx = this->dxCtxSafeObj->Lock();
//        ctx->DrawD2D();
//    }
//
//    void BeginDrawConst() const {
//        auto ctx = this->dxCtxSafeObj->Lock();
//        ctx->DrawD2D();
//    }
//
//private:
//    DxDeviceCtxSafeObj_t* dxCtxSafeObj;
//};
//
//struct RenderParams {
//    // TODO: add wrapper to guard dynamic change interface
//    const DxDeviceCtxSafeObj_t* dxCtxSafeObj;
//    DxLayerStack stack;
//
//    RenderParams()
//        : dxCtxSafeObj{ nullptr }
//        , stack{ dxCtxSafeObj }
//    {}
//};
//
//DxDeviceCtxSafeObj_t dxDeviceCtxSafeObj;
//RenderParams renderParams;
//
//
//
//void Thread1_Render(DxDeviceCtxSafeObj_t::_Locked& dxDeviceCtx) {
//    // work with locked ctx ...
//    renderParams.dxCtxSafeObj = dxDeviceCtx.GetCreator();
//}
//
//void Thread2_Transform(DxDeviceCtxSafeObj_t::_Locked& dxDeviceCtx) {
//    renderParams.stack.BeginDraw();
//}


//struct A {
//    void Test() {
//        int x = 9;
//    };
//
//    void TestConst() const {
//        int x = 9;
//    };
//};
//
//
//struct Wrapper {
//    Wrapper(A& a)
//        : member{ a }
//    {}
//
//    void DoSmth() {
//        this->member.Test();
//        this->member.TestConst();
//    }
//
//    void DoSmthConst() const {
//        this->member.Test();
//        this->member.TestConst();
//    }
//
//private:
//    A member;
//};

#include <Helpers/FunctionTraits.hpp>

enum class SignalType {
    Multi,
    Once,
};

namespace details {
    template <SignalType _SignalType, typename _Fn>
    class SignalBase {
    };


    template <SignalType _SignalType, typename _Ret, typename... _Args>
    class SignalBase<_SignalType, _Ret(_Args...)> {
    public:
        SignalBase() {}
        SignalBase(std::function<_Ret(_Args...)> handler)
        {
            handlers.push_back(handler);
        }

        // [Not thread safe]
        SignalBase& Add(std::function<_Ret(_Args...)> handler) {
            handlers.push_back(handler);
            return *this;
        }

        SignalBase& AddFinish(std::function<_Ret(_Args...)> finishHandler) {
            finishHandlers.push_back(finishHandler);
            return *this;
        }

        void Clear() {
            finishHandlers.clear();
            handlers.clear();
        }

        bool IsTriggeredAtLeastOnce() const {
            return triggerCounter;
        }

        // Qualify as const to use in lambda captures by value
        _Ret operator() (_Args... args) const { // NOTE: _Args must be trivial copyable
            for (auto& handler : handlers) {
                if (handler) {
                    handler(args...);
                }
            }
            for (auto& finishHandler : finishHandlers) {
                if (finishHandler) {
                    finishHandler(args...);
                }
            }

            triggerCounter++;

            if (_SignalType == SignalType::Once && (handlers.size() || finishHandlers.size())) {
                finishHandlers.clear();
                handlers.clear();
            }
        }

    private:
        //std::mutex mx; // We cannot use mutex as mebmer because its copy constructor removed
        mutable std::atomic<int> triggerCounter = 0; // default copy Ctor deleted for std::atomic
        mutable std::vector<std::function<_Ret(_Args...)>> handlers;
        mutable std::vector<std::function<_Ret(_Args...)>> finishHandlers;
    };
}

template <typename _Fn, SignalType _SignalType = SignalType::Multi>
using Signal = details::SignalBase<_SignalType, _Fn>;

template <typename _Fn>
using SignalOnce = details::SignalBase<SignalType::Once, _Fn>;


int main() {
    //A a;
    //Wrapper wrapper(a);
    //wrapper.DoSmthConst();

    //std::_Get_function_impl<void(int)>::type

    //HELPERS_NS::FunctionTraits<void(*)()>::;

    Signal<void(int), SignalType::Once> signal;

    signal(1);
    return 0;
}


//auto dxDeviceCtx = dxDeviceCtxSafeObj.Lock();
//Thread1_Render(dxDeviceCtx);
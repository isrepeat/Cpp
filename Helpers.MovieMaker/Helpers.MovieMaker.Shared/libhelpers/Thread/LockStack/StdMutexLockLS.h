#pragma once
#include "ILockLS.h"
#include "..\LockInspectorScope.h"
#include "..\InspectableLocks.h"

#include <mutex>
#include <memory>

namespace thread {
    class StdMutexLock : protected ILock {
    public:
        StdMutexLock(thread::mutex_std_insp &mtx);
        StdMutexLock(std::unique_ptr<thread::mutex_std_insp> &mtx);
        StdMutexLock(std::shared_ptr<thread::mutex_std_insp> &mtx);
        virtual ~StdMutexLock();

    protected:
        void Lock() override;
        void Unlock() override;

    private:
        std::mutex &mtx;
        LockInspectorScope inpectScope;
        std::unique_lock<std::mutex> lock;
    };
}
#include "pch.h"
#include "StdMutexLockLS.h"
#include "..\LockInspector.h"

namespace thread {
    StdMutexLock::StdMutexLock(thread::mutex_std_insp &mtx)
        : mtx(mtx), inpectScope(&mtx), lock(mtx)
    {
    }

    StdMutexLock::StdMutexLock(std::unique_ptr<thread::mutex_std_insp> &mtx)
        : mtx(*mtx), inpectScope(&mtx), lock(*mtx)
    {
    }

    StdMutexLock::StdMutexLock(std::shared_ptr<thread::mutex_std_insp> &mtx)
        : mtx(*mtx), inpectScope(&mtx), lock(*mtx)
    {
    }

    StdMutexLock::~StdMutexLock() {
    }

    void StdMutexLock::Lock() {
        LockInspector::Instance()->OnLock(&this->mtx);
        this->mtx.lock();
    }

    void StdMutexLock::Unlock() {
        this->mtx.unlock();
        LockInspector::Instance()->OnUnlock(&this->mtx);
    }
}
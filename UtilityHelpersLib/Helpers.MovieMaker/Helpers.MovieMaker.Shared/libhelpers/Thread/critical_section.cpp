#include "pch.h"
#include "critical_section.h"
#include "..\HSystem.h"

#include <thread>

namespace thread {
    critical_section::critical_section(DWORD spinCount) {
        InitializeCriticalSectionEx(&this->cs, spinCount, 0);
#ifdef _DEBUG
        this->ownerId = 0;
#endif // _DEBUG
    }

    critical_section::~critical_section() {
        LockInspector::Instance()->OnLockDestructor(&this->cs);
        DeleteCriticalSection(&this->cs);
    }

    void critical_section::lock() {
        LockInspector::Instance()->OnLock(&this->cs);
        EnterCriticalSection(&this->cs);
#ifdef _DEBUG
        this->own();
#endif // _DEBUG
    }

    void critical_section::unlock() {
#ifdef _DEBUG
        this->unown();
#endif // _DEBUG
        LeaveCriticalSection(&this->cs);
        LockInspector::Instance()->OnUnlock(&this->cs);
    }

#ifdef _DEBUG
    bool critical_section::owned() const {
        bool res = this->ownerId == GetCurrentThreadId();
        return res;
    }
#endif // _DEBUG

#ifdef _DEBUG
    void critical_section::own() {
        // to prevent recursive locking
        hAssert(!this->owned());
        this->ownerId = GetCurrentThreadId();
    }

    void critical_section::unown() {
        this->ownerId = 0;
    }
#endif // _DEBUG
}
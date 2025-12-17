#include "pch.h"
#include "LockInspector.h"
#include "LockInspector\LockInspectorImpl.h"

//#define USE_LockInspector

LockInspector *LockInspector::Instance() {
    static LockInspector instance;
    return &instance;
}

void LockInspector::OnLock(void *csAddress) {
#ifdef USE_LockInspector
    LockInspectorImpl::Instance()->OnLock(csAddress);
#endif // USE_LockInspector

}

void LockInspector::OnUnlock(void *csAddress) {
#ifdef USE_LockInspector
    LockInspectorImpl::Instance()->OnUnlock(csAddress);
#endif // USE_LockInspector

}

void LockInspector::OnLockDestructor(void *csAddress) {
#ifdef USE_LockInspector
    LockInspectorImpl::Instance()->OnLockDestructor(csAddress);
#endif // USE_LockInspector

}

void LockInspector::SetCurrentThreadForBreak() {
#ifdef USE_LockInspector
    LockInspectorImpl::Instance()->SetCurrentThreadForBreak();
#endif // USE_LockInspector

}

void LockInspector::ResetThreadForBreak() {
#ifdef USE_LockInspector
    LockInspectorImpl::Instance()->ResetThreadForBreak();
#endif // USE_LockInspector

}

LockInspector::LockInspector() {
}
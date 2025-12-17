#pragma once

// class for inspection of lock/unlock order for detecting dead locks
class LockInspector {
public:
    static LockInspector *Instance();

    void OnLock(void *csAddress);
    void OnUnlock(void *csAddress);

    void OnLockDestructor(void *csAddress);

    void SetCurrentThreadForBreak();
    void ResetThreadForBreak();

private:
    LockInspector();
};
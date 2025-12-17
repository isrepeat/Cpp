#pragma once
#include "LockInspValue.h"

#include <thread>

enum class LockInspEventType {
    Unknown,
    Lock,
    Unlock,
    Destructor,
};

class LockInspEvent {
public:
    LockInspEvent();
    LockInspEvent(void *lockAddress, LockInspEventType type);

    LockInspEventType GetType() const;
    std::thread::id GetThreadId() const;

    LockInspValue DetachValue();
    LockInspValue &GetValue();
    const LockInspValue &GetValue() const;

private:
    LockInspEventType type;
    std::thread::id threadId;
    LockInspValue value;
};
#include "pch.h"
#include "LockInspEvent.h"

LockInspEvent::LockInspEvent()
    : type(LockInspEventType::Unknown)
{}

LockInspEvent::LockInspEvent(void *lockAddress, LockInspEventType type)
    : value(lockAddress), type(type)
{
    this->threadId = std::this_thread::get_id();
}

LockInspEventType LockInspEvent::GetType() const {
    return this->type;
}

std::thread::id LockInspEvent::GetThreadId() const {
    return this->threadId;
}

LockInspValue LockInspEvent::DetachValue() {
    return std::move(this->value);
}

LockInspValue &LockInspEvent::GetValue() {
    return this->value;
}

const LockInspValue &LockInspEvent::GetValue() const {
    return this->value;
}
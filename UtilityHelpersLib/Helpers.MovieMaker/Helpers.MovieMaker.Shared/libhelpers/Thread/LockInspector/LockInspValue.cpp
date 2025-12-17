#include "pch.h"
#include "LockInspValue.h"

#include <Windows.h>

LockInspValue::LockInspValue()
    : lockAddress(nullptr)
{}

LockInspValue::LockInspValue(void *lockAddress)
    : lockAddress(lockAddress)
{
    this->InitStackTrace();
}

bool LockInspValue::operator==(const LockInspValue &other) const {
    bool equ = this->lockAddress == other.lockAddress && this->stackTrace == other.stackTrace;
    return equ;
}

bool LockInspValue::operator!=(const LockInspValue &other) const {
    return !LockInspValue::operator==(other);
}

void LockInspValue::InitStackTrace() {
    HANDLE process;
    unsigned short frames;
    void *stackTraceTmp[512];

    process = GetCurrentProcess();

    frames = CaptureStackBackTrace(0, ARRAYSIZE(stackTraceTmp), stackTraceTmp, NULL);

    // + 2 to remove LockInspEvent::InitStackTrace() and LockInspEvent::LockInspEvent() records
    this->stackTrace = std::vector<void *>(stackTraceTmp + 2, stackTraceTmp + frames);
}
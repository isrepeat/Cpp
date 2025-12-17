#include "pch.h"
#include "LockInspectorScope.h"
#include "LockInspector.h"

LockInspectorScope::LockInspectorScope(void *csAddress)
    : csAddress(csAddress)
{
    LockInspector::Instance()->OnLock(this->csAddress);
}

LockInspectorScope::~LockInspectorScope() {
    LockInspector::Instance()->OnUnlock(this->csAddress);
}
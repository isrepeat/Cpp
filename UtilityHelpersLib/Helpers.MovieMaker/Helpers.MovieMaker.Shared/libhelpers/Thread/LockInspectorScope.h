#pragma once

class LockInspectorScope {
public:
    LockInspectorScope(void *csAddress);
    ~LockInspectorScope();

private:
    void *csAddress;
};
#pragma once

#include <vector>

struct LockInspValue {
    void *lockAddress;
    std::vector<void *> stackTrace; // visual studio can show function names in debug by void pointer

    LockInspValue();
    LockInspValue(void *lockAddress);

    bool operator==(const LockInspValue &other) const;
    bool operator!=(const LockInspValue &other) const;

private:
    void InitStackTrace();
};
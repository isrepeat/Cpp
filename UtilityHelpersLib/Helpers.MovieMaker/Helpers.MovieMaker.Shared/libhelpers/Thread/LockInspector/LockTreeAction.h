#pragma once
#include "LockInspTreeNode.h"
#include "LockInspEvent.h"

enum class LockTreeActionType {
    None,
    Merge,
    Delete,
};

class LockTreeAction {
public:
    LockTreeActionType type;

    struct {
        LockInspTreeNode tree;
    } mergeData;

    struct {
        LockInspEvent evt;
    } deleteData;

    LockTreeAction();
};
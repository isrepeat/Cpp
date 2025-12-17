#pragma once
#include "LockInspTreeNode.h"
#include "LockInspEvent.h"

class LockInspThreadTree {
public:
    LockInspTreeNode tree;
    LockInspTreeNode *curSubTree;
    int depth;

    LockInspThreadTree();

    void Add(LockInspEvent &&item);
    bool AtRoot() const;
};
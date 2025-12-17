#include "pch.h"
#include "LockInspThreadTree.h"

LockInspThreadTree::LockInspThreadTree()
    : curSubTree(nullptr), depth(0)
{
}

void LockInspThreadTree::Add(LockInspEvent &&item) {
    switch (item.GetType()) {
    case LockInspEventType::Lock: {
        if (this->AtRoot()) {
            this->curSubTree = &tree;
        }
        this->curSubTree = this->curSubTree->Add(item.DetachValue());
        this->depth++;
        break;
    }
    case LockInspEventType::Unlock: {
        if (this->AtRoot()) {
            this->tree.failed = true;
            this->tree.failReason += "--!--many unlocks \r\n";
            return;
        }

        if (item.GetValue().lockAddress != this->curSubTree->value.lockAddress) {
            // bad locks
            this->tree.failed = true;
            this->tree.failReason += "--!--lock order not matches unlock order \r\n";
            return;
        }

        this->curSubTree = this->curSubTree->parent;
        this->depth--;
        break;
    }
    /*case LockInspEventType::Destructor: {
        break;
    }*/
    default:
        break;
    }

    //if (item.lock) {
    //    if (this->AtRoot()) {
    //        this->curSubTree = &tree;
    //    }

    //    this->curSubTree = this->curSubTree->AddNode(item.csAddress);
    //    this->depth++;
    //}
    //else {
    //    if (this->AtRoot()) {
    //        this->tree.failed = true;
    //        this->tree.failReason += "--!--many unlocks \r\n";
    //        return;
    //    }

    //    if (item.csAddress != this->curSubTree->csAddress) {
    //        // bad locks
    //        this->tree.failed = true;
    //        this->tree.failReason += "--!--lock order not matches unlock order \r\n";
    //        return;
    //    }

    //    this->curSubTree->CaptureStack();
    //    this->curSubTree = this->curSubTree->parent;
    //    this->depth--;
    //}
}

bool LockInspThreadTree::AtRoot() const {
    return !this->curSubTree;
}
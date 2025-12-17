#include "pch.h"
#include "LockInspTreeNode.h"

#include <assert.h>

LockInspTreeNode::BranchIterator::BranchIterator(const LockInspTreeNode &tree) {
    this->rootVal = tree.value;
    this->AddTreeIterators(tree);
}

bool LockInspTreeNode::BranchIterator::operator==(const BranchIterator &other) const {
    if (this->nodeIterators.size() == other.nodeIterators.size()) {
        for (size_t i = 0; i < this->nodeIterators.size(); i++) {
            bool tmpSame =
                this->nodeIterators[i].curIt == other.nodeIterators[i].curIt &&
                this->nodeIterators[i].endIt == other.nodeIterators[i].endIt;

            if (!tmpSame) {
                return false;
            }
        }

        return true;
    }

    return false;
}

bool LockInspTreeNode::BranchIterator::operator!=(const BranchIterator &other) const {
    return !BranchIterator::operator==(other);
}

std::vector<void *> LockInspTreeNode::BranchIterator::GetBranch() const {
    std::vector<void *> branch;

    this->GetBranch(branch);
    return branch;
}

void LockInspTreeNode::BranchIterator::GetBranch(std::vector<void *> &values) const {
    values.push_back(this->rootVal.lockAddress);

    for (auto it = this->nodeIterators.begin(); it != this->nodeIterators.end(); ++it) {
        values.push_back((*it).curIt->value.lockAddress);
    }
}

std::vector<LockInspValue> LockInspTreeNode::BranchIterator::GetBranchWithStack() const {
    std::vector<LockInspValue> branch;

    this->GetBranchWithStack(branch);
    return branch;
}

void LockInspTreeNode::BranchIterator::GetBranchWithStack(std::vector<LockInspValue> &values) const {
    values.push_back(this->rootVal);

    for (auto it = this->nodeIterators.begin(); it != this->nodeIterators.end(); ++it) {
        values.push_back((*it).curIt->value);
    }
}

bool LockInspTreeNode::BranchIterator::IsEnd() const {
    return this->nodeIterators.empty();
}

void LockInspTreeNode::BranchIterator::Next() {
    if (!this->nodeIterators.empty()) {
        auto it = &this->nodeIterators.back();

        ++it->curIt;

        if (it->curIt == it->endIt) {
            this->nodeIterators.pop_back();
            this->Next();
        }
        else {
            this->AddTreeIterators(*it->curIt);
        }
    }
}

void LockInspTreeNode::BranchIterator::AddTreeIterators(const LockInspTreeNode &tree) {
    const LockInspTreeNode *curTree = &tree;

    while (!curTree->nodes.empty()) {
        ItPair it;

        it.curIt = curTree->nodes.begin();
        it.endIt = curTree->nodes.end();

        this->nodeIterators.push_back(it);
        curTree = &curTree->nodes.front();
    }
}

LockInspTreeNode::LockInspTreeNode()
    : parent(nullptr), failed(false)//, hash(0)
{
}

LockInspTreeNode::LockInspTreeNode(const LockInspTreeNode &other, LockInspTreeNode *parent)
    : value(other.value),
    parent(parent),
    failed(other.failed), failReason(other.failReason)
{
    for (auto &i : other.nodes) {
        this->nodes.push_back(LockInspTreeNode(i, this));
    }
}

LockInspTreeNode::LockInspTreeNode(LockInspTreeNode &&other)
    : value(std::move(other.value)),
    parent(std::move(other.parent)),
    nodes(std::move(other.nodes)),
    failed(std::move(other.failed)), failReason(std::move(other.failReason))
{
    for (auto &i : this->nodes) {
        i.parent = this;
    }

    other.parent = nullptr;
    other.failed = false;
}

LockInspTreeNode &LockInspTreeNode::operator=(const LockInspTreeNode &other) {
    if (this != &other) {
        this->value = other.value;
        this->parent = nullptr;
        this->failed = other.failed;
        this->failReason = other.failReason;

        for (auto &i : other.nodes) {
            this->nodes.push_back(LockInspTreeNode(i, this));
        }
    }

    return *this;
}

LockInspTreeNode &LockInspTreeNode::operator=(LockInspTreeNode &&other) {
    if (this != &other) {
        this->value = std::move(other.value);
        this->parent = std::move(other.parent);
        this->nodes = std::move(other.nodes);
        this->failed = std::move(other.failed);
        this->failReason = std::move(other.failReason);

        for (auto &i : this->nodes) {
            i.parent = this;
        }

        other.parent = nullptr;
        other.failed = false;
    }

    return *this;
}

LockInspTreeNode *LockInspTreeNode::Add(LockInspValue &&val) {
    if (this->value.lockAddress) {
        LockInspTreeNode newNode;

        newNode.value = std::move(val);
        newNode.parent = this;

        this->nodes.push_back(std::move(newNode));

        return &this->nodes.back();
    }
    else {
        this->value = std::move(val);
        return this;
    }
}

bool LockInspTreeNode::RemoveLockRecursive(const LockInspValue &val) {
    bool thisRemoved = false;

    if (this->value.lockAddress == val.lockAddress) {
        *this = LockInspTreeNode();
        thisRemoved = true;
    }
    else {
        this->nodes.remove_if([&](LockInspTreeNode &v) {
            return v.RemoveLockRecursive(val);
        });
    }

    return thisRemoved;
}

bool LockInspTreeNode::MergeToNodes(LockInspTreeNode &other) {
    return this->MergeToNodesHelper(*this, other);
}

LockInspTreeNode::BranchIterator LockInspTreeNode::GetFirstBranchIterator() const {
    return LockInspTreeNode::BranchIterator(*this);
}

void LockInspTreeNode::CheckParent(LockInspTreeNode *parent) {
    assert(this->parent == parent);

    for (auto &i : this->nodes) {
        i.CheckParent(this);
    }
}

bool LockInspTreeNode::MergeToNodesHelper(LockInspTreeNode &_this, LockInspTreeNode &other) {
    bool updated = false;
    bool merged = false;

    for (auto &n : _this.nodes) {
        if (n.value == other.value) {
            merged = true;

            for (auto &i : other.nodes) {
                bool updTmp = LockInspTreeNode::MergeToNodesHelper(n, i);
                updated = updated || updTmp;
            }

            break;
        }
    }

    if (!merged) {
        updated = true;
        _this.nodes.push_back(std::move(other));
        _this.nodes.back().parent = &_this;
        //_this.nodes.back().ResetParents(_this);
    }

    return updated;
}

void LockInspTreeNode::ResetParents(LockInspTreeNode &parent) {
    this->parent = &parent;

    for (auto &n : this->nodes) {
        n.ResetParents(*this);
    }
}

//// http://stackoverflow.com/questions/919612/mapping-two-integers-to-one-in-a-unique-and-deterministic-way
//void LockInspTree::AddHash(void *csAddress) {
//    // Szudzik's function
//    uint64_t a = this->hash;
//    uint64_t b = reinterpret_cast<uint64_t>(csAddress);
//    uint64_t result = a >= b ? a * a + a + b : a + b * b;
//    this->hash = result;
//}
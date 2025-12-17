#pragma once
#include "LockInspValue.h"

#include <list>
#include <string>
#include <vector>

class LockInspTreeNode {
public:
    class BranchIterator {
    public:
        BranchIterator(const LockInspTreeNode &tree);

        bool operator==(const BranchIterator &other) const;
        bool operator!=(const BranchIterator &other) const;

        std::vector<void *> GetBranch() const;
        void GetBranch(std::vector<void *> &values) const;

        std::vector<LockInspValue> GetBranchWithStack() const;
        void GetBranchWithStack(std::vector<LockInspValue> &values) const;

        bool IsEnd() const;
        void Next();

    private:
        struct ItPair {
            std::list<LockInspTreeNode>::const_iterator curIt;
            std::list<LockInspTreeNode>::const_iterator endIt;
        };
        LockInspValue rootVal;
        std::vector<ItPair> nodeIterators;

        void AddTreeIterators(const LockInspTreeNode &tree);
    };

    LockInspValue value;
    LockInspTreeNode *parent;
    std::list<LockInspTreeNode> nodes;
    bool failed;
    std::string failReason;

    LockInspTreeNode();
    LockInspTreeNode(const LockInspTreeNode &other, LockInspTreeNode *parent = nullptr);
    LockInspTreeNode(LockInspTreeNode &&other);

    LockInspTreeNode &operator=(const LockInspTreeNode &other);
    LockInspTreeNode &operator=(LockInspTreeNode &&other);

    LockInspTreeNode *Add(LockInspValue &&val);

    bool RemoveLockRecursive(const LockInspValue &val);

    bool MergeToNodes(LockInspTreeNode &other);

    BranchIterator GetFirstBranchIterator() const;

    void CheckParent(LockInspTreeNode *parent = nullptr);

private:

    bool MergeToNodesHelper(LockInspTreeNode &_this, LockInspTreeNode &other);
    void ResetParents(LockInspTreeNode &parent);
};
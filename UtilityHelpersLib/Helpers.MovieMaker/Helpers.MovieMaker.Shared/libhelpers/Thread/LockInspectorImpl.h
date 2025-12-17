#pragma once

#include <vector>
#include <list>
#include <thread>
#include <mutex>
#include <map>
#include <condition_variable>

struct LockInspLkItem {
    std::thread::id threadId;
    bool lock;
    void *csAddress;
};

class LockInspTree {
public:
    void *csAddress;
    LockInspTree *parent;
    std::list<LockInspTree> nodes;
    /*uint64_t hash;*/
    bool failed;
    std::string failReason;

    LockInspTree();
    LockInspTree *AddNode(void *csAddress);
    /*void AddHash(void *csAddress);*/

private:
};

class LockInspThreadTree {
public:
    LockInspTree tree;
    LockInspTree *curSubTree;
    int depth;

    LockInspThreadTree();

    void Add(const LockInspLkItem &item);
    bool AtRoot() const;
};

class LockInspectorImpl {
public:
    static LockInspectorImpl *Instance();

    void OnLock(void *csAddress);
    void OnUnlock(void *csAddress);

    void SetCurrentThreadForBreak();
    void ResetThreadForBreak();

private:
    LockInspectorImpl();
    ~LockInspectorImpl();

    bool exit;
    std::thread workThread;
    std::mutex lkItemsMtx;
    std::condition_variable lkItemsCv;
    std::vector<LockInspLkItem> lkItems;

    // breakThread
    std::mutex breakThreadIdMtx;
    std::thread::id breakThreadId;

    // used only by workThread
    // no lock is needed
    std::vector<LockInspLkItem> curLkItems;

    std::mutex treeMtx;
    std::map<std::thread::id, LockInspThreadTree> curThreadTree;
    //std::map<uint32_t, LockInspTree> lockTreeMap;

    std::thread treeThread;
    std::vector<LockInspTree> curTreeItems;
    std::mutex treeItemsMtx;
    std::condition_variable treeItemsCv;
    std::vector<LockInspTree> treeItems;
    LockInspTree lockTree;

    void ThreadMain();
    void ProcessItems();
    void ProcessItem(const LockInspLkItem &item);

    void TreeThreadMain();
    void ProcessTreeItems();
};
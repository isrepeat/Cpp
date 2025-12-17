#pragma once
#include "LockInspThreadTree.h"
#include "LockTreeAction.h"

#include <vector>
#include <thread>
#include <mutex>
#include <map>
#include <atomic>
#include <condition_variable>

//struct LockInspLkItem {
//    std::thread::id threadId;
//    bool lock;
//    void *csAddress;
//};

//class LockInspThreadTree {
//public:
//    LockInspTree tree;
//    LockInspTree *curSubTree;
//    int depth;
//
//    LockInspThreadTree();
//
//    void Add(const LockInspLkItem &item);
//    bool AtRoot() const;
//};

class LockInspectorImpl {
public:
    static LockInspectorImpl *Instance();

    void OnLock(void *csAddress);
    void OnUnlock(void *csAddress);
    void OnLockDestructor(void *csAddress);

    void SetCurrentThreadForBreak();
    void ResetThreadForBreak();

private:
    LockInspectorImpl();
    ~LockInspectorImpl();

    std::atomic_bool exit;

    std::thread lkEventsThread;
    std::mutex lkEventsMtx;
    std::condition_variable lkEventsCv;
    std::vector<LockInspEvent> lkEventsProd;
    std::vector<LockInspEvent> lkEventsCons;

    // breakThread
    std::mutex breakThreadIdMtx;
    std::thread::id breakThreadId;

    std::mutex treeMtx;
    std::map<std::thread::id, LockInspThreadTree> curThreadTree;

    std::thread treeThread;
    std::mutex treeItemsMtx;
    std::condition_variable treeItemsCv;
    std::vector<LockTreeAction> treeItemsProd;
    std::vector<LockTreeAction> treeItemsCons;
    LockInspTreeNode lockTree;

    std::thread treeReportThread;
    std::mutex treeReportMtx;
    std::condition_variable treeReportCv;
    std::vector<LockInspTreeNode> treeReportProd;
    std::vector<LockInspTreeNode> treeReportCons;
    uint64_t lockTreeLastUpdate;

    void ThreadMain();
    void ProcessItem(LockInspEvent &&item);

    void TreeThreadMain();
    void ProcessTreeItems();

    void TreeReportThreadMain();
    bool ItIsTimeToReport();
    void Report(const LockInspTreeNode &lkTree);
};